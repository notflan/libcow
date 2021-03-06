// `Area` is a copyable type that opaquely represents either `Cow` or `Cow::Fake`.
#pragma once

#include <memory>
#include <utility>

#include <cow.hpp>
#include "slice.hpp"

namespace _cow_util {
	struct Area : public Span<unsigned char> {
		Area() = delete;
		
		explicit Area(size_t sz);
		Area(const Area& area);
		Area(Area&& area);

		Area(Cow&& move);
		explicit Area(const Cow& copy);

		inline const Cow* operator->() const { return _area.get(); }
		inline Cow* operator->() { return _area.get(); }

		inline const Cow* operator*() const { return _area.get(); }
		inline Cow* operator*() { return _area.get(); }

		inline operator const Cow&() const { return *_area.get(); }
		inline operator Cow&() { return *_area.get(); }

		inline bool is_clone() const { return dynamic_cast<Cow::Fake*>(_area.get()) != nullptr; }

		inline cow_t* raw() const { return _area->raw(); }

		inline size_t size() const override { return _area->size(); }
		
		~Area();
		protected:
		inline void* area() override { return _area->ptr(); }
		inline const void* area() const override { return _area->ptr(); }

		private:
		const std::unique_ptr<Cow> _area;
	};
}
