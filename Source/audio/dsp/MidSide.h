#pragma once
#include "ProcessorBufferView.h"

namespace dsp
{
	template<typename Float>
	void midSideEncode(Float* const*, int) noexcept;

	template<typename Float>
	void midSideDecode(Float* const*, int) noexcept;

	void midSideEncode(ProcessorBufferView&) noexcept;

	void midSideDecode(ProcessorBufferView&) noexcept;
}