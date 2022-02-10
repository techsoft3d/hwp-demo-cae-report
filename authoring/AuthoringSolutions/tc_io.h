#pragma once
#define _TC_FILE_H

#include <vector>
#include <array>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <memory>

#if _MSC_VER
#include <codecvt>
#include <filesystem>
#if _MSC_VER >= 1900
namespace system_filesystem = std::experimental::filesystem;
#else
namespace system_filesystem = std::tr2::sys;
#endif
#else
#include <boost/filesystem.hpp>
namespace system_filesystem = boost::filesystem;
#endif

namespace TC {

	namespace IO {

		char const * formatf(
			char * fixed_buffer, size_t fixed_buffer_size,
			std::vector<char> & fallback_buffer,
			char const * format, va_list arguments);

		size_t format_time_string(char * out_string, size_t n); // n is buffer size - returns length without terminator

		void outputDebug(char const * str);

		void output(char const * str);


#ifndef _MSC_VER
		class args {
		public:
			args(int &, char **&) {}
			args(int &, char **&, char **&){}
			~args() {}
		};
#else
		class internal_args;
		class args {
		public:
			args(int &, char **&);
			args(int &, char **&, char **&);
			~args();
		private:
			std::unique_ptr<internal_args> _impl;
		};
#endif


#if !defined(_MSC_VER) || _MSC_VER >= 1900
		using path = system_filesystem::path;
		using directory_iterator = system_filesystem::directory_iterator;
#else
		typedef system_filesystem::wpath path;
		typedef system_filesystem::wdirectory_iterator directory_iterator;
#endif

#if !defined(_MSC_VER)
		inline path::string_type utf8_to_path_string(const char * utf8)
		{
			return utf8;
		}
#else
		inline std::wstring utf8_to_wstring(const std::string& str)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.from_bytes(str); // convert UTF-8 string to wstring
		}

		inline std::string wstring_to_utf8(const std::wstring& str)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.to_bytes(str); // convert wstring to UTF-8 string
		}

		inline path::string_type utf8_to_path_string(const char * utf8)
		{
			return utf8_to_wstring(utf8);
		}
#endif

		inline path utf8_to_path(const char * utf8)
		{
			return path(utf8_to_path_string(utf8));
		}

		inline path utf8_to_path(std::string const & utf8_string)
		{
			return path(utf8_to_path_string(utf8_string.c_str()));
		}

		inline bool is_regular_file(path const & p) {
			return system_filesystem::is_regular_file(p);
		}

		inline bool is_directory(path const & p) {
			return system_filesystem::is_directory(p);
		}

		inline bool exists(path const & p) {
			return system_filesystem::exists(p);
		}

		inline time_t last_write_time(path const & p) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
			auto time_point = system_filesystem::last_write_time(p);
			return std::chrono::system_clock::to_time_t(time_point);
#	else
			return system_filesystem::last_write_time(p); // not sure when but this will probably go away some year maybe 2017
#	endif
		}

		inline bool create_directory(path const & p) {
			return system_filesystem::create_directory(p);
		}

		inline path absolute(path const & p) {
#if defined(_MSC_VER) && _MSC_VER >= 1900
			return system_filesystem::absolute(p);
#else
			return system_filesystem::complete(p);
#endif
		}

		inline std::string extension(path const & p) {
#if _MSC_VER
#	if _MSC_VER >= 1900
			return p.extension().u8string();
#	else
			return wstring_to_utf8(p.extension());
#	endif
#else
			return p.extension().string();
#endif
		}

		inline std::string basename(path const & p) {
#if _MSC_VER
#	if _MSC_VER >= 1900
			return p.stem().u8string();
#	else
			return wstring_to_utf8(system_filesystem::basename(p));
#	endif
#else
			return system_filesystem::basename(p);
#endif
		}

		inline uintmax_t remove_all(path const & p) {
			return system_filesystem::remove_all(p);
		}

		inline std::string leaf_string(path const & p)
		{
#if _MSC_VER
#	if _MSC_VER >= 1900
			return p.filename().u8string();
#	else
			return wstring_to_utf8(p.leaf());
#	endif
#else
			return p.leaf().native();
#endif
		}

		inline std::string file_string(path const & p)
		{
#if _MSC_VER
#	if _MSC_VER >= 1900
			return p.u8string();
#	else
			return wstring_to_utf8(p.file_string());
#	endif
#else
			return p.native();
#endif
		}


		inline std::string directory_string(path const & p)
		{
#if _MSC_VER
#	if _MSC_VER >= 1900
			return p.u8string();
#	else
			return wstring_to_utf8(p.directory_string());
#	endif
#else
			return p.native();
#endif
		}

		void generate_unique_name(std::array<char, 32> & chars);

		path unique_path();

		bool create_directory_symlink(path const & target_path, path const & link_path);
#if 0
		{
			try {
				system_filesystem::create_directory_symlink(target_path, link_path);
			}
			catch (...) {
				return false;
			}
			return true;
		}
#endif

		inline bool rename(path const & current_path, path const & new_path)
		{
			try {
				system_filesystem::rename(current_path, new_path);
			}
			catch (...) {
				return false;
			}
			return true;
		}

		inline FILE * fopen(path const & path, char const * mode)
		{
#if _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )
#	if _MSC_VER >= 1900
			return _wfopen(path.wstring().c_str(), utf8_to_wstring(mode).c_str());
#	else
			return _wfopen(path.file_string().c_str(), utf8_to_wstring(mode).c_str());
#	endif
#pragma warning( pop ) 	
#else
			return ::fopen(IO::file_string(path).c_str(), mode);
#endif
		}

		inline bool remove(path const & path)
		{
			return system_filesystem::remove(path);
		}

		inline int fseek64(FILE * file, int64_t offset, int origin)
		{
#ifdef _MSC_VER
			return _fseeki64(file, offset, origin);
#else
			static_assert(sizeof(off64_t) == sizeof(int64_t), "Sizes don't coincide.");
			return fseeko64(file, offset, origin);
#endif
		}


		inline int64_t ftell64(FILE * file)
		{
#ifdef _MSC_VER
			return _ftelli64(file);
#else
			static_assert(sizeof(off64_t) == sizeof(int64_t), "Sizes don't coincide.");
			return ftello64(file);
#endif
		}



		// Multi-reader single writer file access
		class File
		{
		public:
			enum State
			{
				None,
				Again,
				Failed,
				ReadOnly,
				ReadWrite
			};

			enum Position
			{
				Begin,
				Current,
				End
			};

			File();

			File(path const & in_path);

			~File()
			{
				Release();
			}

			inline State GetState() const
			{
				return _state;
			}

			inline bool IsOpen() const
			{
				return (_state == ReadOnly || _state == ReadWrite);
			}

			void SetPath(path const & in_path);

			inline path const & GetPath() const
			{
				return _path;
			}

			void Release();

			inline State AcquireReadOnly() {
				return acquire(true);
			}

			inline State AcquireReadWrite() {
				return acquire(false);
			}

			int64_t Seek(int64_t offset, Position position = Begin); // returns offset from begin

			bool Truncate(); // at current offset return true if it worked

			int64_t Size() const;

			size_t Read(void * data, size_t n_bytes) const;

			size_t Write(const void * data, size_t n_bytes);

		private:

			State acquire(bool read_only);

			path		_path;
			intptr_t	_opaque_handle;
			State		_state;
		};

		class block_cache {

			typedef std::vector<uint8_t> bytes_t;
			struct block_t {
				size_t generation;
				bytes_t bytes;
				inline block_t() : generation(0), bytes() {}
				inline void reset() {
					generation = 0;
					bytes = bytes_t();
				}
			};

			typedef std::vector<block_t> blocks_t;

			int64_t		file_size;
			blocks_t	blocks;
			size_t		n_loaded;
			size_t		generation;

		public:

			enum ConstExpr : size_t {
				block_shift = 20,
				block_size = (1 << block_shift),
				max_n_loaded = 256,
			};

			block_cache() : file_size(0), blocks(), n_loaded(0), generation(0)
			{
			}

			virtual ~block_cache()
			{
			}

			virtual IO::File * get_file() = 0;

			void flush();

			bool acquire(int64_t offset, size_t size);

			void read(int64_t offset, size_t total_size, uint8_t * buffer);

		};

	}


}

