#include "../../include/util/file.h"

File::File(const std::string& filename)
  : _stdin(filename == ""), _filename(filename)
{}
File::File(const File& f) = default;

File& File::operator=(const File& f) = default;

File::~File() = default;

