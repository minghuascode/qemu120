
#include <clcpp/clcpp.h>

#include <cstdio>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include "test.cpp"


class StdFile : public clcpp::IFile {
public:
        StdFile(const char* filename) {
                m_FP = fopen(filename, "rb");
                if (m_FP == 0) {
                        return;
                }
        }
        ~StdFile() {
                if (m_FP != 0) {
                        fclose(m_FP);
                }
        }
        bool IsOpen() const {
                return m_FP != 0;
        }
        bool Read(void* dest, clcpp::size_type size) {
                return fread(dest, 1, size, m_FP) == size;
        }
private:
        FILE* m_FP;
};

class Malloc : public clcpp::IAllocator {
        void* Alloc(clcpp::size_type size) {
                return malloc(size);
        }
        void Free(void* ptr) {
                free(ptr);
        }
};

extern void clcppInitGetType(const clcpp::Database* db);


int main(int argc, char *argv[]) 
{
        StdFile file("module_export.cppbin");
        if (!file.IsOpen()) {
                return 1;
	}

        Malloc allocator;
        clcpp::Database db;
        if (!db.Load(&file, &allocator, 0)) {
                return 1;
	}

        clcppInitGetType(&db);

	clcpp::Name na = db.GetName("outer::inner1_s");
	const clcpp::Type * e0 = clcpp::GetType<outer::inner2_s>();
	const clcpp::Class *e1 = clcpp::GetType<outer::inner3_c>()->AsClass();

  return 0;
}


