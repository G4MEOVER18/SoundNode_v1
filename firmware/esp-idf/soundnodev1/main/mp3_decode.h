#pragma once
#include <string>
#include <vector>

void mp3_decoder_start(const std::vector<std::string>* tracks);
void mp3_decoder_set_playing(bool on);
void mp3_decoder_next();
void mp3_decoder_prev();
bool mp3_decoder_is_playing();
std::string mp3_decoder_current_path();
void mp3_decoder_set_volume(int pct); // 0..100
int  mp3_decoder_volume();
