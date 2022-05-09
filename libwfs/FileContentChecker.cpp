#include "FileContentChecker.h"
#include <iostream>
#include <fcntl.h>
#include <openssl/sha.h>
#include <macgyver/Base64.h>
#include <spine/Convenience.h>

using SmartMet::Plugin::WFS::FileContentChecker;

FileContentChecker::FileContentChecker(std::size_t max_size, int debug_level)
  : max_size(max_size)
  , debug_level(debug_level)
{
}

FileContentChecker::~FileContentChecker() = default;

bool FileContentChecker::check_file_hash(const std::string& fn)
{
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  char buffer[1024];
  int fd = open(fn.c_str(), O_RDONLY);
  if (fd >= 0) {
    int len;
    std::size_t size = 0;
    while ((len = read(fd, buffer, sizeof(buffer))) > 0 and (max_size == 0 or size < max_size)) {
      SHA256_Update(&ctx, buffer, len);
      size += len;
    }
    close(fd);
    if ((max_size == 0) or (size < max_size)) {
      unsigned char md[SHA256_DIGEST_LENGTH];
      SHA256_Final(md, &ctx);
      const std::string digest = Fmi::Base64::encode(std::string(md, md + SHA256_DIGEST_LENGTH));
      auto it = file_hash_map.find(fn);
      if (it == file_hash_map.end()) {
	file_hash_map[fn] = digest;
	return true;
      } else if (it->second == digest) {
	if (debug_level >= 1) {
	  std::cout << SmartMet::Spine::log_time_str()
		    << ": [WFS] [DEBUG]: " << fn << " unchanged"
		    << std::endl;
	}
        // SHA256 digest has not changed since last check: no need to reread
	return false;
      } else {
	// File SHA256 digest has changed: should reread
	it->second = digest;
	return true;
      }
    } else {
      // File is too long: let's ignore it
      return false;
    }
  } else {
    return false;
  }
}

void FileContentChecker::remove_file_hash(const std::string& fn)
{
  file_hash_map.erase(fn);
}
