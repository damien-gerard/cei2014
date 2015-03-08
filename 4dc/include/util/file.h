#ifndef FILE_H
#define FILE_H

#include <string>
#include <fstream>

class File
{
  public:
    File(const std::string& filename);
    File(const File&);
    ~File();
    
    File& operator=(const File&);
    
    inline bool isStdin() const {return _stdin;}
    inline const std::string& filename() const {return _filename;}
  protected:
  private:
    bool _stdin;
    std::string _filename;
};

#endif // FILE_H
