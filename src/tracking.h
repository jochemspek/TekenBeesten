#ifndef __tracking__h__
#define __tracking__h__
struct Track
{
    int id;	// tracking id
    float x;	// range [-0.5, 0.5]
    float y;	// range [-0.5, 0.5]
};

typedef std::list<Track> Tracks;

Tracks processCamera();
int initCamera(bool show);	// show -> show video and tracking

// Background subtraction
#define TRACK_HISTORY	1000		// Number of frames in background history
// Threshold on the squared Mahalanobis distance between the pixel and the model to decide whether
// a pixel is well described by the background model. This parameter does not affect the background update.
#define TRACK_THRESHOLD	16
#define TRACK_BLUR	7		// Image blurring factor, for noise reduction

// Blob detection
#define TRACK_MIN_BLOB_AREA	2000	// Minimal size of tracked blob (in pixels)
#define TRACK_MAX_BLOB_AREA	100000	// Maximal size of tracked blob (in pixels)

#define	TRACK_DISTANCE		5.0	// Max distance to determine when a track and a blob match
#define TRACK_INACTIVE		10	// Max nunber of frames a track can be inactive

// Camera perspective settings
#define TRACK_NEAR	300		// Camera image position on floor (in cm)
#define	TRACK_FAR	600
#endif
