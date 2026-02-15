#ifndef VIDEO_H
#define VIDEO_H

extern int CurrentVideoFrame;
extern int VideoFrameCount;
extern int VideoWidth;
extern int VideoHeight;
extern int VideoSurface;
extern int VideoFilePos;
extern bool VideoPlaying;

void UpdateVideoFrame();

#endif // !VIDEO_H
