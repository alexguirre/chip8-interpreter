#pragma once
#include <functional>
#include <optional>

namespace c8
{
	template<class T>
	using optional_ref = std::optional<std::reference_wrapper<T>>;

	template<class T>
	using optional_cref = std::optional<std::reference_wrapper<const T>>;
}
