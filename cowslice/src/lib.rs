
#![allow(dead_code)]

mod ffi;

pub mod error;
pub use error::Error;

#[cfg(test)]
mod tests {
    #[test]
    fn link_works() {
	unsafe {
	    let raw_h = super::ffi::cow_create(100);
	    assert_ne!(raw_h as usize, 0);
	    println!("Raw handle pointer is {:p}", raw_h);
	    super::ffi::cow_free(raw_h);
	}
	unsafe {
	    assert_eq!(super::ffi::cow_err(), 1, "There was an error in `cow_free`");
	}
    }
}
