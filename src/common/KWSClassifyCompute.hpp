#pragma once

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "dsp/basic_math_functions.h"
#include "dsp/statistics_functions.h"

using namespace arm_cmsis_stream;

extern "C" {
extern void node_softmax(float *in, size_t blockSize);
}

class KWSClassifyCompute
{
	static constexpr size_t nbLabels = 12;
	
public:

  KWSClassifyCompute() = delete;
  KWSClassifyCompute(int historyLength):historySize_(historyLength)
  {
	history.resize(historySize_ + 1);
	for (auto &v : history) 
	{
		v.resize(nbLabels, 0.0f);
	}
  }

  void resume()
  {
	for (auto &v : history) 
	{
			std::fill(v.begin(), v.end(), 0.0f);
	}
  }

 int operator()(const float *t)
 {
		memcpy(buf, t, nbLabels * sizeof(float));
		// softmax
		node_softmax(buf, nbLabels);
		// add array to history
		for (int i = historySize_ - 1; i > 0; i--) {
			history[i] = std::move(history[i - 1]);
		}
		history[0] = std::vector<float>(buf, buf + nbLabels);

		memset(buf, 0, nbLabels * sizeof(float));
		for (const auto &v : history) {
			arm_add_f32(v.data(), buf, buf, nbLabels);
		}

		// find max
		uint32_t index;
		float res;
		arm_max_f32(buf, nbLabels, &res, &index);
		return index;
	}

    const float* latest()
    {
        return history[0].data();
    }

private:
	uint32_t lastRec{11};
	float buf[nbLabels];
	std::vector<std::vector<float>> history;
	size_t historySize_;
};