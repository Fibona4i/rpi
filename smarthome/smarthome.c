using namespace std;
int Debug;

#include "smarthome.h"

char *ini_path(char *path)
{
	static char *def_path = NULL;

	if (path)
	{
		if (def_path)
			free(path);
		def_path = strdup(path);
	}

	return def_path;
}

static int init_debug(void)
{
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cout << "Can't load config: " << ini_path(NULL) << endl;
		return -1;
	}

	Debug = ini_file.GetInteger("debug", "enabled", -1);
	if (Debug < 0)
	{
		cerr << LINE_INFO << "Couldn't read Debug value" << endl;
		return -1;
	}

	return 0;
}

void *fifo_free(void *data)
{
	struct vfifo_t *vfifo = (struct vfifo_t *)data;
	struct fifo_free_ctx *fifo_ctx = &vfifo->fifo_ctx;
	char *buffer = new char(vfifo->fifo_ctx.pipe_size);

	while(1)
	{
		if (fifo_ctx->need_clean)
		{
			read(fifo_ctx->fd, buffer, fifo_ctx->pipe_size);
			fifo_ctx->need_clean = 0;
		}
		usleep(100*1000);
	}

	delete buffer;

	return NULL;
}

static void frm_if_exist(char *path, ...)
{
	va_list ap;
	int res = 0;

	va_start(ap, path);
	while (path) {
		if (access(path, F_OK) != -1)
			res |= remove(path);
		path = va_arg(ap, char *);
	}
	va_end(ap);

	if (res)
		cerr << LINE_INFO << "Couldn't remove src/dst video files" << endl;
}

static int create_fifos(struct finf_t *finf, ...)
{
	va_list ap;
	int res = 0;

	va_start(ap, finf);
	while (finf) {
		frm_if_exist(finf->path, NULL);
		res |= mkfifo(finf->path, 0666);
		finf = va_arg(ap, struct finf_t *);
	}
	va_end(ap);

	if (res)
		cerr << LINE_INFO << "Couldn't remove src/dst video files" << endl;
	return res;
}



string path_def_get(void)
{
	ssize_t count;
	char result[PATH_MAX];
	static string path = NULL;

	if (path.empty())
	{
		count = readlink("/proc/self/exe", result, PATH_MAX);
		path = string(result, (count > 0) ? count : 0);
		path = path.substr(0, path.find_last_not_of('/'));
	}

	return path;
}

static int read_def_path(struct vfifo_t *vfifo)
{
	string path_def, path, tmp;
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cout << LINE_INFO << "Can't load config: " << ini_path(NULL) << endl;
		return -1;
	}

	path_def = path_def_get();

	tmp = ini_file.Get("path", "vscr", 0);
	if (tmp.empty()) {
		cout << LINE_INFO << "Can't read [path]:vscr " << endl;
		return -1;
	}
	path = path_def + tmp;
	vfifo->vsrc.path = strdup(path.c_str());

	tmp = ini_file.Get("path", "vdst", "");
	if (tmp.empty()) {
		cout << LINE_INFO << "Can't read [path]:vdst " << endl;
		return -1;
	}
	path = path_def + tmp;
	vfifo->vdst.path = strdup(path.c_str());

	return path_def.empty() || !vfifo->vsrc.path || !vfifo->vdst.path;
}

static int init_vfiles(struct vfifo_t *vfifo)
{
	int res = 0;

	res |= create_fifos(&vfifo->vsrc, &vfifo->vdst, NULL);

	//hack: we must aleways write into dst file and ignore SIGPIPE (error if noone read from it)
	res |= open(vfifo->vdst.path, O_RDONLY | O_NONBLOCK);
	signal(SIGPIPE, SIG_IGN);

	vfifo->vdst.fd = open(vfifo->vdst.path, O_WRONLY | O_NONBLOCK | O_ASYNC | O_NOATIME);
	vfifo->vsrc.fd = open(vfifo->vsrc.path, O_RDONLY);

	if (res || (vfifo->vsrc.fd | vfifo->vdst.fd) < 0) {
		cerr << LINE_INFO << "Couldn't open src/dst video files" << endl;
		return -1;
	}

	return 0;
}

static int init_vvals(struct vfifo_t *vfifo)
{
	string path_def, path;
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cout << "Can't load config: " << ini_path(NULL) << endl;
		return -1;
	}

	vfifo->v_ctx.duration = ini_file.GetInteger("video", "preduf_time", -1);
	vfifo->v_ctx.sec_size = ini_file.GetInteger("video", "one_sec_size", -1);
	vfifo->v_ctx.buf = new char(vfifo->fifo_ctx.pipe_size);
	vfifo->fifo_ctx.pipe_size = fcntl(0, F_GETPIPE_SZ) + 1;

	if (vfifo->fifo_ctx.pipe_size == -1 || vfifo->v_ctx.duration == -1 || vfifo->v_ctx.sec_size == -1 ||
		!vfifo->v_ctx.buf)
	{
		cerr << LINE_INFO << "Couldn't init def values" << endl;
		return -1;
	}
	debug("pipe_size = " << vfifo->fifo_ctx.pipe_size);

	return 0;
}

static int gpio_thread_crate(struct gpio_t *gpio)
{
	pthread_t thread_gpio_r;

	if (pthread_create(&thread_gpio_r, NULL, gpio_read, gpio))
	{
		cerr << LINE_INFO << "Couln't open gpio thread" << endl;
		return -1;
	}

	return 0;
}

static int clean_fifo_thread_crate(struct fifo_free_ctx *fifo_ctx)
{
	pthread_t thread_clean_fifo_r;

	if (pthread_create(&thread_clean_fifo_r, NULL, fifo_free, fifo_ctx))
	{
		cerr << LINE_INFO << "Couln't open fifo_free thread" << endl;
		return -1;
	}

	return 0;
}

static int init_vbuf(struct ring_buf **buf, int duration, int sec_size)
{
	struct ring_buf *first_vbuf, *tmp_vbuf, *vbuf = *buf;

	/* init video pre-buffer */
	for (int i=0; i < duration; i++)
	{
		vbuf = (struct ring_buf *)malloc(sizeof(struct ring_buf));
		vbuf->head = vbuf->curr = (char *)malloc(sec_size * 1024);
		vbuf->full_size = sec_size * 1024;
		vbuf->next = tmp_vbuf;
		tmp_vbuf = vbuf;

		if (!vbuf || !vbuf->head)
		{
			cerr << LINE_INFO << "Couldn't alloc vbuf" << endl;
			return -1;
		}

		if (!i)
			first_vbuf = vbuf;
	}
	first_vbuf->next = vbuf;

	return 0;
}

static int read_fifo(struct vfifo_t *vfifo)
{
	while (1)
	{
		vfifo->v_ctx.r_bytes = read(vfifo->vsrc.fd, vfifo->v_ctx.buf, vfifo->fifo_ctx.pipe_size);
		if (vfifo->v_ctx.r_bytes < 0 && errno == EINTR)
			continue;
		if (vfifo->v_ctx.r_bytes <= 0)
		{
			cerr << LINE_INFO << "Couldn't read video src" << endl;
			return -1;
		}
		break;
	}

	return 0;
}

static int write_fifo(struct vfifo_t *vfifo)
{
	int w_size = write(vfifo->vdst.fd, vfifo->v_ctx.buf, vfifo->v_ctx.r_bytes);

	if (w_size == -1 && !vfifo->fifo_ctx.need_clean)
		vfifo->fifo_ctx.need_clean = 1;

	return 0;
}

static int update_ring_buf(struct ring_buf *vbuf, struct vfifo_t *vfifo)
{
	time_t tm;
	static time_t prev_time = 0;

	if ((tm = time(NULL)) != prev_time)
	{
		prev_time = tm;
		vbuf = vbuf->next;
		vbuf->curr = vbuf->head;
	}
	if ((vbuf->curr - vbuf->head + vfifo->v_ctx.r_bytes) >= vbuf->full_size)
	{
		cerr << LINE_INFO << "Video bitrade is too high" << endl;
		return -1;
	}
	memcpy(vbuf->curr, vfifo->v_ctx.buf, vfifo->v_ctx.r_bytes);
	vbuf->curr += vfifo->v_ctx.r_bytes;

	return 0;
}
static inline int need_start_saving(struct gpio_t *gpio)
{
	return (gpio->stat && !gpio->in_progr);
}

static inline int need_stop_saving(struct gpio_t *gpio)
{
	return (!gpio->stat && gpio->in_progr);
}

static inline int is_saving_in_progr(struct gpio_t *gpio)
{
	return (gpio->stat && gpio->in_progr);
}

static string get_time_str(void)
{
	char buf[80];
	time_t now = time(0);

	strftime(buf, sizeof(buf), "CAM_%Y-%m-%d:%M:%S.ts", localtime(&now));

	return buf;
}

static void start_video_saving(struct ring_buf *vbuf, struct fsave_t *fsave, struct gpio_t *gpio)
{
	int i;
	string fsave_name;
	struct ring_buf *tmp_vbuf;

	fsave->path_tmp = path_def_get() + "tmp/";
	fsave->name = get_time_str();
	fsave_name = fsave->path_tmp + fsave->name;

	fsave->file.open(fsave_name.c_str(), ios::out | ios::binary);
	if (fsave->file.is_open())
	{
		cerr << LINE_INFO << "Couldn't open file [" << fsave->path_tmp << fsave->name <<
			"] for video saving" << endl;
	}
	gpio->in_progr = 1;

	for (i=0, tmp_vbuf=vbuf->next; tmp_vbuf != vbuf->next || !i++; tmp_vbuf=tmp_vbuf->next)
		fsave->file.write(tmp_vbuf->head, tmp_vbuf->curr - tmp_vbuf->head);
}

static void stop_video_saving(struct fsave_t *fsave, struct gpio_t *gpio)
{
	string old_file = fsave->path_tmp + fsave->name;
	string new_file = fsave->path + fsave->name;

	if (fsave->file.is_open())
		fsave->file.close();
	rename(old_file.c_str(), new_file.c_str());
	gpio->in_progr = 0;
}

static void video_write(struct fsave_t *fsave, struct video_ctx *v_ctx)
{
	fsave->file.write(v_ctx->buf, v_ctx->r_bytes);
}

static void uninit(struct ring_buf *vbuf, struct vfifo_t *vfifo, struct fsave_t *fsave, struct gpio_t *gpio)
{
	if (vbuf->curr) {
		int i;
		struct ring_buf *tmp_vbuf;

		for (i=0, tmp_vbuf=vbuf->next; tmp_vbuf != vbuf->next || !i++; tmp_vbuf=tmp_vbuf->next)
		{
			free(tmp_vbuf->head);
			free(tmp_vbuf);
		}
	}

	if (vfifo->vdst.fd > 0)
		close(vfifo->vdst.fd);
	if (vfifo->vsrc.fd > 0)
		close(vfifo->vsrc.fd);
	if (vfifo->v_ctx.buf)
		delete vfifo->v_ctx.buf;
	if (fsave->file.is_open())
		fsave->file.close();
	if (vfifo->vsrc.path)
		free(vfifo->vsrc.path);
	if (vfifo->vdst.path)
		free(vfifo->vdst.path);
	if (gpio->gpio_in)
		delete gpio->gpio_in;
	if (gpio->gpio_out)
		delete gpio->gpio_out;
}

int main(int argc, char *argv[])
{
	struct gpio_t gpio = {};
	struct fsave_t fsave = {};
	struct vfifo_t vfifo = {};
	struct ring_buf *vbuf = NULL;

	if (init_debug())
		goto Exit;

	if(argc != 2)
	{
		cerr << "Usage:\n nameprog /path/to/ini_file" << endl;
		goto Exit;
	}

	if (!ini_path(argv[1]))
		goto Exit;

	if (read_def_path(&vfifo))
		goto Exit;

	if (init_vfiles(&vfifo))
		goto Exit;

	if (init_vvals(&vfifo))
		goto Exit;

	if (gpio_thread_crate(&gpio))
		goto Exit;

	if (clean_fifo_thread_crate(&vfifo.fifo_ctx))
		goto Exit;

	if (init_vbuf(&vbuf, vfifo.v_ctx.duration, vfifo.v_ctx.sec_size))
		goto Exit;

	while(1)
	{
		if (read_fifo(&vfifo) && write_fifo(&vfifo))
			goto Exit;

		if (update_ring_buf(vbuf, &vfifo))
			goto Exit;

		if (need_start_saving(&gpio))
			start_video_saving(vbuf, &fsave, &gpio);
		else if (need_stop_saving(&gpio))
			stop_video_saving(&fsave, &gpio);
		else if (is_saving_in_progr(&gpio))
			video_write(&fsave, &vfifo.v_ctx);
	}

Exit:
	uninit(vbuf, &vfifo, &fsave, &gpio);
	return 0;
}
