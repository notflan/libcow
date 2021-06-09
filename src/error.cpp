#include <array>

#include <cow.h>

#include "error.h"

namespace _cow_error {
	constexpr const size_t SIZE = (size_t)_COW_ERR_SIZE;

	consteval inline void setmsg(std::array<const char*, SIZE>& ar, enum cow_err_kind kind, const char* msg)
	{
		ar[kind] = msg;
	}

	consteval inline std::array<const char*, SIZE> gen_msg_table()
	{
		std::array<const char*, SIZE> ret;

		setmsg(ret, COW_ERR_UNKNOWN, "unknown error");
		setmsg(ret, COW_ERR_NONE, "success");
		setmsg(ret, COW_ERR_FDCREATE, "failed to create shmfd (memfd_create())");
		setmsg(ret, COW_ERR_SIZE, "failed to set shmfd size (ftruncate())");
		setmsg(ret, COW_ERR_MAP, "failed to map shmfd (mmap())");

		return ret;
	}

	const std::array<const char*, SIZE> _cow_message_table = gen_msg_table();
}
using namespace _cow_error;

extern "C" {

	enum cow_err_kind cow_err() {
		return _cow_last_error;
	}
	internal void _cow_set_err(enum cow_err_kind kind) {
		_cow_last_error = kind;
	}
	const char* const * cow_err_msg(enum cow_err_kind kind)
	{
		auto idx = ((size_t)kind);
		if ( idx >= SIZE ) return nullptr;
		else return &_cow_message_table[idx];
	}
}
