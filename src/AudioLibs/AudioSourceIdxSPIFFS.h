#pragma once
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include "AudioLogger.h"
#include "AudioBasic/StrExt.h"
#include "AudioTools/AudioSource.h"
#include "AudioLibs/SDIndex.h"

namespace audio_tools {

/**
 * @brief ESP32 AudioSource for AudioPlayer using an SPI Flash File System (SPIFFS) as data source.
 *  This library makes use of the SPIFFS library provided as part of the ESP32-IDF or the Arduino Framework
 * 
 * @author Phil Schatzmann, Bobby Lindsey
 * @copyright GPLv3
 */
class AudioSourceSPIFFS : public AudioSource {
public:
  /// Default constructor
  AudioSourceSPIFFS(const char *startFilePath = "/", const char *ext = ".mp3", int chipSelect = PIN_CS, bool setupIndex=true) {
    start_path = startFilePath;
    extension = ext;
    setup_index = setupIndex;
    cs = chipSelect;
  }

  virtual void begin() override {
    LOGD(LOG_METHOD);
    static bool is_spiffs_setup = false;
    if (!is_spiffs_setup) {
      if (!SPIFFS.begin(cs)) {
        LOGE("SPIFFS.begin cs=%d failed", cs);
        return;
      }
      is_spiffs_setup = true;
    }
    idx.begin(start_path, extension, file_name_pattern, setup_index);
    idx_pos = 0;
  }

  virtual Stream *nextStream(int offset = 1) override {
    LOGI("nextStream: %d", offset);
    return selectStream(idx_pos + offset);
  }

  virtual Stream *selectStream(int index) override {
    LOGI("selectStream: %d", index);
    idx_pos = index;
    file_name = idx[index];
    if (file_name==nullptr) return nullptr;
    LOGI("Using file %s", file_name);
    file = SPIFFS.open(file_name);
    return file ? &file : nullptr;
  }

  virtual Stream *selectStream(const char *path) override {
    file.close();
    file = SPIFFS.open(path);
    file_name = file.name();
    LOGI("-> selectStream: %s", path);
    return file ? &file : nullptr;
  }

  /// Defines the regex filter criteria for selecting files. E.g. ".*Bob
  /// Dylan.*"
  void setFileFilter(const char *filter) { file_name_pattern = filter; }

  /// Provides the current index position
  int index() { return idx_pos; }

  /// provides the actual file name
  const char *toStr() { return file_name; }

  // provides default setting go to the next
  virtual bool isAutoNext() { return true; }

  /// Allows to "correct" the start path if not defined in the constructor
  virtual void setPath(const char *p) { start_path = p; }

protected:
  SDIndex<fs::SPIFFSFS,fs::File> idx{SPIFFS};
  // SDIndex<fs::SDFS,fs::File> idx{SD};
  File file;
  size_t idx_pos = 0;
  const char *file_name;
  const char *extension = nullptr;
  const char *start_path = nullptr;
  const char *file_name_pattern = "*";
  bool setup_index = true;
  int cs;


};

} // namespace audio_tools
