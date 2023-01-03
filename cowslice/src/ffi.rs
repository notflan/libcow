//! Interfaces with the C library
use core::ffi::c_void;

/// Raw `cow_t*`
///
/// This is, essentially, a pointer to the origin (`void*`) pointer.
/// So is represented as `*mut RawHandle == void**`.
pub type RawHandle = *mut c_void;

extern "C" {
    pub fn cow_create(sz: usize) -> *mut RawHandle;
    pub fn cow_free(cow: *mut RawHandle);
    pub fn cow_clone(cow: *const RawHandle) -> *mut RawHandle;

    pub fn cow_is_fake(cow: *const RawHandle) -> i32;
    pub fn cow_size(cow: *const RawHandle) -> usize;

    pub fn cow_err() -> i32;
    pub fn cow_err_msg(kind: i32) -> *const *const u8;
}

#[inline(always)] pub unsafe fn cow_area_mut(cow: *mut RawHandle) -> *mut c_void
{
    *cow
}
#[inline(always)] pub unsafe fn cow_area(cow: *const RawHandle) -> *const c_void
{
    (*cow) as *const _
}
