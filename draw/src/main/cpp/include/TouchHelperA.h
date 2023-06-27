#pragma once

//小的分辨率在前面
bool Touch_Init(float x, float y, bool readOnly);

void Touch_Close();

void Touch_Down(float x, float y);

void Touch_Move(float x, float y);

void Touch_Up();

