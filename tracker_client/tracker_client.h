/**
 * tracker_client.h — types for pulling tracker pose (name, time, position) from libsurvive.
 * Include this from your own C code when linking to tracker_client or libsurvive.
 */
#ifndef TRACKER_CLIENT_H
#define TRACKER_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/** Position in meters (x, y, z). */
typedef struct TrackerPos {
	double x, y, z;
} TrackerPos;

/** Rotation as quaternion (w, x, y, z). */
typedef struct TrackerQuat {
	double w, x, y, z;
} TrackerQuat;

/** One tracker update: name, serial, time, pose. */
typedef struct TrackerUpdate {
	const char *name;    /**< Short name, e.g. "WM0", "TR0" */
	const char *serial;  /**< Serial number, e.g. "LHR-XXXXXX" */
	double      time;    /**< Timecode (seconds) of this pose */
	TrackerPos  pos;     /**< Position (meters) */
	TrackerQuat rot;     /**< Rotation quaternion (w,x,y,z) */
} TrackerUpdate;

/**
 * Callback type: invoked for each pose update.
 * Return 0 to keep running, non-zero to stop the event loop.
 */
typedef int (*TrackerPoseCallback)(const TrackerUpdate *update, void *user);

#ifdef __cplusplus
}
#endif

#endif
