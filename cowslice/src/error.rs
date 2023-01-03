use super::*;
use std::{error, fmt};
use std::ffi::CStr;

/// A `libcow` error object.
#[derive(Debug, Clone, PartialEq, Eq, Hash, PartialOrd, Ord)]
#[repr(transparent)]
pub struct Error(i32);

impl Error
{
    /// The last error in `libcow`.
    #[inline(always)] pub fn last() -> Self
    {
	Self(unsafe {ffi::cow_err()})
    }
}

impl error::Error for Error{}
impl fmt::Display for Error
{
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result
    {
	const DEFAULT: &'static [u8] = b"unknown\0"; // must be `nul`'d
	let cstr = unsafe {
	    let ptr = ffi::cow_err_msg(self.0);
	    if ptr.is_null() {
		CStr::from_bytes_with_nul_unchecked(DEFAULT)
	    } else {
		let ptr = *ptr;
		if ptr.is_null() {
		    CStr::from_bytes_with_nul_unchecked(DEFAULT)
		} else {
		    CStr::from_ptr(ptr as *const i8)
		}
	    }
	};
	write!(f, "{}", cstr.to_string_lossy())
    }
}

