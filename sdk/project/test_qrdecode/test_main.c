#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/itu.h"
#include "SDL/SDL.h"

#include "dbgutil.h"



//	-------------------------------------------------------
//	DEFines
//	-------------------------------------------------------

#define IN_FILE					"A:/123.png"								// Output file name

// QR decode defines
struct result_info {
	int		file_count;
	int		id_count;
	int		decode_count;

	unsigned int	load_time;
	unsigned int	identify_time;
	unsigned int	total_time;
};

static struct quirc *decoder;

static int want_verbose = 1;
static int want_cell_dump = 0;
void add_result(struct result_info *sum, struct result_info *inf);
void print_result(const char *name, struct result_info *info);
void ITEQRDecode();
// QR decode defines

static char decode_result[200];

static ITUScene scene;

static bool OnPress(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
	static char msg[256] = { 0 };
	ITUTextBox* txtbox = (ITUTextBox*)ituSceneFindWidget(&scene, "TextBox");
	ITUIcon* icon = (ITUIcon*)ituSceneFindWidget(&scene, "Icon");
	if (icon)
	{
		ITUButton* btn = (ITUButton*)widget;

		if ((ev == ITU_EVENT_KEYDOWN && arg1 == SDLK_RETURN))
		{
			return true;
		}
		else if (ev == ITU_EVENT_MOUSEDOWN)
		{
			if (!strcmp(widget->name, "BtnClear"))
			{
				strcpy(msg, "QR Clear");
				ituTextSetString(txtbox, msg);
				ituIconReleaseSurface(icon);
			}
			else
			{
				ITEQRDecode();
				strcpy(msg, decode_result);
				ituTextSetString(txtbox, msg);
				ituIconLoadPngFile(icon, IN_FILE);
			}
			return true;
		}
		else if (ev == ITU_EVENT_MOUSEUP)
		{
			if (ituButtonIsPressed(btn))
			{
				return true;
			}
		}
	}
	return false;
}

void output_data(const struct quirc_data *data)
{
	memset(decode_result, 0, sizeof(decode_result));
	strcpy(decode_result, data->payload);
}

int scan_dir(const char *path, const char *filename, struct result_info *info)
{
	DIR *d = opendir(path);
	struct dirent *ent;
	int count = 0;

#ifdef _WIN32
	if (!d) {
		fprintf(stderr, "%s: opendir: %s\n", path, strerror(errno));
		return -1;
	}
#endif
	printf("%s:\n", path);

	while ((ent = readdir(d))) {
		if (ent->d_name[0] != '.') {
			char fullpath[1024];
			struct result_info sub;

			snprintf(fullpath, sizeof(fullpath), "%s/%s",
				path, ent->d_name);
			if (test_scan(fullpath, &sub) > 0) {
				add_result(info, &sub);
				count++;
			}
		}
	}

	closedir(d);

	if (count > 1) {
		print_result(filename, info);
		puts("");
	}

	return count > 0;
}

void add_result(struct result_info *sum, struct result_info *inf)
{
	sum->file_count += inf->file_count;
	sum->id_count += inf->id_count;
	sum->decode_count += inf->decode_count;

	sum->load_time += inf->load_time;
	sum->identify_time += inf->identify_time;
	sum->total_time += inf->total_time;
}

void print_result(const char *name, struct result_info *info)
{
	puts("----------------------------------------"
		"---------------------------------------");
	printf("%s: %d files, %d codes, %d decoded (%d failures)",
		name, info->file_count, info->id_count, info->decode_count,
		(info->id_count - info->decode_count));
	if (info->id_count)
		printf(", %d%% success rate",
		(info->decode_count * 100 + info->id_count / 2) /
		info->id_count);
	printf("\n");
	printf("Total time [load: %u, identify: %u, total: %u]\n",
		info->load_time,
		info->identify_time,
		info->total_time);
	if (info->file_count)
		printf("Average time [load: %u, identify: %u, total: %u]\n",
		info->load_time / info->file_count,
		info->identify_time / info->file_count,
		info->total_time / info->file_count);
}

static int scan_file(const char *path, const char *filename, struct result_info *info)
{
	int(*loader)(struct quirc *, const char *);
	int len = strlen(filename);
	const char *ext;
	struct timespec tp;
	unsigned int start;
	unsigned int total_start;
	int ret;
	int i;

	while (len >= 0 && filename[len] != '.')
		len--;
	ext = filename + len + 1;
	if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0)
		loader = load_jpeg;
	else if (strcasecmp(ext, "png") == 0)
		loader = load_png;
	else
		return 0;

	ret = loader(decoder, path);

	if (ret < 0) {
		fprintf(stderr, "%s: load failed\n", filename);
		return -1;
	}

	quirc_end(decoder);

	info->id_count = quirc_count(decoder);
	for (i = 0; i < info->id_count; i++) 
	{
		struct quirc_code code;
		struct quirc_data data;

		quirc_extract(decoder, i, &code);

		if (!quirc_decode(&code, &data))
			info->decode_count++;
	}

	printf("  %-30s: %5u %5u %5u %5d %5d\n", filename,
		info->load_time,
		info->identify_time,
		info->total_time,
		info->id_count, info->decode_count);

	if (want_cell_dump || want_verbose)
	{
		for (i = 0; i < info->id_count; i++)
		{
			struct quirc_code code;

			quirc_extract(decoder, i, &code);
			if (want_cell_dump)
			{
				dump_cells(&code);
				printf("\n");
			}

			if (want_verbose)
			{
				struct quirc_data data;
				quirc_decode_error_t err =
					quirc_decode(&code, &data);

				if (err)
				{
					printf("  ERROR: %s\n\n",
						quirc_strerror(err));
				}
				else 
				{
					printf("  Decode successful:\n");
					dump_data(&data);
					// Copy decode data into global char array
					output_data(&data);
					// Copy end
					printf("\n");
				}
			}
		}
	}

	info->file_count = 1;
	return 1;
}

int test_scan(const char *path, struct result_info *info)
{
	int len = strlen(path);
	struct stat st;
	const char *filename;

	memset(info, 0, sizeof(*info));

	while (len >= 0 && path[len] != '/')
		len--;
	filename = path + len + 1;

#ifdef _WIN32
	if (access(path, F_OK) == -1){
		fprintf(stderr, "%s: lstat: %s\n", path, strerror(errno));
		return -1;
	}
#endif
	/*if (lstat(path, &st) < 0) {
		fprintf(stderr, "%s: lstat: %s\n", path, strerror(errno));
		return -1;
	}*/

	//f (S_ISREG(st.st_mode))
		return scan_file(path, filename, info);

	//if (S_ISDIR(st.st_mode))
	//	return scan_dir(path, filename, info);

	return 0;
}

void ITEQRDecode()
{
	struct result_info sum;
	int count = 0;
	int i;

	decoder = quirc_new();
	if (!decoder) {
		perror("quirc_new");
		return -1;
	}

	printf("  %-30s  %17s %11s\n", "", "Time (ms)", "Count");
	printf("  %-30s  %5s %5s %5s %5s %5s\n",
		"Filename", "Load", "ID", "Total", "ID", "Dec");
	puts("----------------------------------------"
		"---------------------------------------");

	memset(&sum, 0, sizeof(sum));

	struct result_info info;
	if (test_scan(IN_FILE, &info) > 0)
		add_result(&sum, &info);

	if (count > 1)
		print_result("TOTAL", &sum);

	quirc_destroy(decoder);
}

//	-------------------------------------------------------
//	Main
//	-------------------------------------------------------

void* TestFunc(void* arg)
{
	itpInit();

	// wait mouting USB storage
#ifndef _WIN32
	sleep(3);
#endif

	//// Decode
	//struct result_info sum;
	//int count = 0;
	//int i;

	//decoder = quirc_new();
	//if (!decoder) {
	//	perror("quirc_new");
	//	return -1;
	//}

	//printf("  %-30s  %17s %11s\n", "", "Time (ms)", "Count");
	//printf("  %-30s  %5s %5s %5s %5s %5s\n",
	//	"Filename", "Load", "ID", "Total", "ID", "Dec");
	//puts("----------------------------------------"
	//	"---------------------------------------");

	//memset(&sum, 0, sizeof(sum));

	//struct result_info info;
	//if (test_scan(IN_FILE, &info) > 0)
	//	add_result(&sum, &info);

	//if (count > 1)
	//	print_result("TOTAL", &sum);

	//quirc_destroy(decoder);
	//// Decode

	//	-------------------------------------------------------
	//	SDL start
	//	-------------------------------------------------------
	SDL_Window *window;
	SDL_Event ev;
	int done, delay;
	uint32_t tick, dblclk;
	ITUWidget* widget;
	ITUIcon* icon;

	// wait mouting USB storage
#ifndef _WIN32
	sleep(3);
#endif

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}
	window = SDL_CreateWindow("ITU Button Test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, 0);
	if (!window)
	{
		printf("Couldn't create 800x600 window: %s\n",
			SDL_GetError());
		SDL_Quit();
		exit(2);
	}

	// init itu
	ituLcdInit();

#ifdef CFG_M2D_ENABLE
	ituM2dInit();
#else
	ituSWInit();
#endif

	ituFtInit();
	ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/DroidSans.ttf", ITU_GLYPH_8BPP);

	// load itu file
	tick = SDL_GetTicks();

	ituSceneInit(&scene, NULL);
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/qrdecode.itu");

	printf("loading time: %dms\n", SDL_GetTicks() - tick);

	scene.upKey = SDLK_UP;
	scene.downKey = SDLK_DOWN;

	// customize button behavior
	widget = ituSceneFindWidget(&scene, "BtnShow");
	if (widget)
		ituWidgetSetOnPress(widget, OnPress);

	widget = ituSceneFindWidget(&scene, "BtnClear");
	if (widget)
		ituWidgetSetOnPress(widget, OnPress);
	// customize button behavior

	/* Watch keystrokes */
	done = dblclk = 0;
	while (!done)
	{
		tick = SDL_GetTicks();

		while (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
			case SDL_FINGERDOWN:
				printf("\nOuch, you touch me at: (%d, %d)", ev.tfinger.x, ev.tfinger.y);
				ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
				switch (ev.key.keysym.sym)
				{
				case SDLK_TAB:
					ituSceneFocusNext(&scene);
					break;
				}
				break;

			case SDL_FINGERUP:
				ituSceneUpdate(&scene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
				break;

			case SDL_KEYDOWN:
				ituSceneUpdate(&scene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
				switch (ev.key.keysym.sym)
				{
				case SDLK_TAB:
					ituSceneFocusNext(&scene);
					break;
				}
				break;

			case SDL_KEYUP:
				ituSceneUpdate(&scene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
				break;

			case SDL_MOUSEBUTTONDOWN:
			{
				uint32_t t = SDL_GetTicks();
				if (t - dblclk <= 300)
				{
					ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
					dblclk = 0;
				}
				else
				{
					ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
					dblclk = t;
				}
			}
			break;

			case SDL_MOUSEBUTTONUP:
				ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
				break;

			case SDL_QUIT:
				done = 1;
				break;
			}
		}

		if (ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0))
		{
			ituSceneDraw(&scene, ituGetDisplaySurface());
			ituFlip(ituGetDisplaySurface());
		}

		delay = 33 - (SDL_GetTicks() - tick);
		if (delay > 0)
		{
			SDL_Delay(delay);
		}
	}

	SDL_Quit();
	// SDL end

	return NULL;
}

