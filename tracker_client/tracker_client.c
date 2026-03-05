/**
 * tracker_client — pull Vive tracker position, time, and name from libsurvive.
 *
 * Built with libsurvive; run from the build dir so plugins are found (or set SURVIVE_PLUGINS).
 *
 * Usage:
 *   sudo ./tracker_client              — print each pose to stdout
 *   sudo ./tracker_client --no-print   — run event loop only (for integration)
 */
#include <stdio.h>
#include <string.h>
#include <survive_api.h>
#include <os_generic.h>
#include "tracker_client.h"

static volatile int keep_running = 1;
static int no_print = 0;

#ifdef __linux__
#include <signal.h>
#include <stdlib.h>
static void int_handler(int dummy) {
	(void)dummy;
	if (keep_running == 0) exit(-1);
	keep_running = 0;
}
#endif

static void log_fn(SurviveSimpleContext *actx, SurviveLogLevel logLevel, const char *msg) {
	(void)logLevel;
	fprintf(stderr, "(%7.3f) tracker_client: %s\n", survive_simple_run_time(actx), msg);
}

/** Safe string for printf; libsurvive can return NULL for name/serial. */
static const char *safe_str(const char *s) {
	return (s && *s) ? s : "(null)";
}

/** Fill a TrackerUpdate from a pose event. */
static void fill_tracker_update(const struct SurviveSimplePoseUpdatedEvent *ev, TrackerUpdate *out) {
	if (!ev) return;
	out->name   = survive_simple_object_name(ev->object);
	out->serial = survive_simple_serial_number(ev->object);
	out->time   = ev->time;
	out->pos.x  = (double)ev->pose.Pos[0];
	out->pos.y  = (double)ev->pose.Pos[1];
	out->pos.z  = (double)ev->pose.Pos[2];
	out->rot.w  = (double)ev->pose.Rot[0];
	out->rot.x  = (double)ev->pose.Rot[1];
	out->rot.y  = (double)ev->pose.Rot[2];
	out->rot.z  = (double)ev->pose.Rot[3];
}

/** Default callback: print pose; return 0 to keep running. */
static int default_callback(const TrackerUpdate *u, void *user) {
	(void)user;
	if (!u) return 0;
	if (no_print) return 0;
	printf("%s %s (%7.3f): pos [%.6f, %.6f, %.6f] quat [%.6f, %.6f, %.6f, %.6f]\n",
	       safe_str(u->name), safe_str(u->serial), u->time,
	       u->pos.x, u->pos.y, u->pos.z,
	       u->rot.w, u->rot.x, u->rot.y, u->rot.z);
	fflush(stdout);
	return 0;
}

/**
 * Run the libsurvive event loop and invoke callback for each pose update.
 * Returns when callback returns non-zero or shutdown.
 * Pass argc/argv from main so libsurvive gets the real command line (e.g. --force-calibrate).
 */
int tracker_client_run(int argc, char **argv, TrackerPoseCallback callback, void *user) {
	SurviveSimpleContext *actx;
	SurviveSimpleEvent event = {0};
	int c = (argc > 0) ? argc : 1;
	char **av = (argv && argv[0]) ? argv : (char *[]){ "tracker_client", NULL };

	if (!callback) callback = default_callback;

	actx = survive_simple_init_with_logger(c, av, log_fn);
	if (!actx) return -1;

	survive_simple_start_thread(actx);

	for (const SurviveSimpleObject *it = survive_simple_get_first_object(actx); it != 0;
	     it = survive_simple_get_next_object(actx, it)) {
		printf("Found '%s'\n", safe_str(survive_simple_object_name(it)));
	}

	while (keep_running && survive_simple_wait_for_event(actx, &event) != SurviveSimpleEventType_Shutdown) {
		if (event.event_type == SurviveSimpleEventType_PoseUpdateEvent) {
			const struct SurviveSimplePoseUpdatedEvent *pose_ev = survive_simple_get_pose_updated_event(&event);
			if (pose_ev) {
				TrackerUpdate u;
				fill_tracker_update(pose_ev, &u);
				if (callback(&u, user) != 0) break;
			}
		}
	}

	survive_simple_close(actx);
	return 0;
}

int main(int argc, char **argv) {
#ifdef __linux__
	signal(SIGINT, int_handler);
	signal(SIGTERM, int_handler);
	signal(SIGKILL, int_handler);
#endif
	for (int i = 1; i < argc && argv[i]; i++) {
		if (strcmp(argv[i], "--no-print") == 0) no_print = 1;
	}
	return tracker_client_run(argc, argv, NULL, NULL) == 0 ? 0 : 1;
}
