#ifndef VIDEO_H
#define VIDEO_H

extern int currentVideoFrame;
extern int videoFrameCount;
extern int videoWidth;
extern int videoHeight;
extern int videoSurface;
extern int videoFilePos;
extern bool videoPlaying;

void UpdateVideoFrame();

#endif // !VIDEO_H
