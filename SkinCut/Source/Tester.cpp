#include "Tester.hpp"

#include "Cutter.hpp"
#include "Entity.hpp"
#include "Util.hpp"
#include "StopWatch.hpp"
#include "Structs.hpp"
#include "RenderTarget.hpp"

#include <sstream>


using namespace SkinCut;
using namespace SkinCut::Math;


// Number of runs for performance test
constexpr auto cNumTestRuns = 100;


std::vector<Tester::Sample> Tester::CreateSamples(SampleLocations& locations, std::vector<float>& lengths, std::wstring setName)
{
	std::vector<Sample> samples{};
	if (lengths.size() != locations.size()) {
		return samples;
	}

	for (uint32_t i = 0; i < locations.size(); ++i) {
		float hlength = 0.5f * lengths[i];

		Vector2 p = locations[i].first;		// position
		Vector2 d = locations[i].second;	// direction

		Vector2 p0 = p - d * hlength;
		Vector2 p1 = p + d * hlength;

		std::wstringstream wss;
		wss << setName << " " << i + 1;

		samples.push_back(std::make_tuple(wss.str(), p0, p1));
	}

	return samples;
}

void Tester::RunTest(std::shared_ptr<Cutter> cutter, std::vector<Sample>& samples, Vector2& resolution, Vector2& window, Matrix& projection, Matrix& view)
{
	for (auto& sample : samples) {
		std::array<long long, 5> stageTime = {}; // init values to zero

		for (uint32_t run = 0; run < (uint32_t)cNumTestRuns; ++run) {
			StopWatch sw;

			Intersection ix0 = cutter->Intersect(std::get<1>(sample), resolution, window, projection, view);
			Intersection ix1 = cutter->Intersect(std::get<2>(sample), resolution, window, projection, view);

			Quadrilateral cutQuad;
			std::list<Link> cutLine;
			std::vector<Edge*> cutEdges;
			std::shared_ptr<RenderTarget> patch;

			sw.Start("1");
			ix0.Model->FormCutline(ix0, ix1, cutLine, cutQuad);
			sw.Stop("1");

			sw.Start("2");
			cutter->GenPatch(cutLine, ix0.Model, patch);
			sw.Stop("2");

			sw.Start("3");
			cutter->DrawPatch(cutLine, ix0.Model, patch);
			sw.Stop("3");

			sw.Start("4");
			ix0.Model->FuseCutline(cutLine, cutEdges);
			sw.Stop("4");

			sw.Start("5");
			ix0.Model->OpenCutLine(cutEdges, cutQuad);
			sw.Stop("5");

			stageTime[0] += sw.ElapsedTime("1");
			stageTime[1] += sw.ElapsedTime("2");
			stageTime[2] += sw.ElapsedTime("3");
			stageTime[3] += sw.ElapsedTime("4");
			stageTime[4] += sw.ElapsedTime("5");

			ix0.Model->Reload();
		}


		std::wstringstream wss;
		wss << std::get<0>(sample) << std::endl;

		double total_time = 0.0;
		for (uint32_t ti = 0; ti < stageTime.size(); ti++)
		{
			double stageTimeAvg = stageTime[ti] / (double)cNumTestRuns / 1000.0;
			wss << stageTimeAvg << std::endl;
			total_time += stageTimeAvg;
		}

		wss << total_time << std::endl;
		Util::ConsoleMessageW(wss.str());
	}
}


void Tester::Test(std::shared_ptr<Cutter> cutter, Vector2 resolution, Vector2 window)
{
	Matrix proj {
		3.047189f, 0.00000000f, 0.000000000f, 0.0f,
		0.000000f, 5.67128229f, 0.000000000f, 0.0f,
		0.000000f, 0.00000000f, 1.005025150f, 1.0f,
		0.000000f, 0.00000000f,-0.100502513f, 0.0f
	};

	Matrix view{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 5.0f, 1.0f
	};

	// screen-space locations and normalized direction vectors based on manual samples
	std::vector<std::pair<Vector2, Vector2>> sampleLocations = {
		std::make_pair(Vector2(638.0f, 175.0f), Vector2(0.999650240f, 0.026445773f)),
		std::make_pair(Vector2(754.0f, 342.0f), Vector2(-0.138322249f, 0.990387261f)),
		std::make_pair(Vector2(618.0f, 618.0f), Vector2(0.899437010f, 0.437050372f)),
		std::make_pair(Vector2(692.0f, 375.0f), Vector2(0.474099845f, 0.880471110f)),
		std::make_pair(Vector2(582.0f, 346.0f), Vector2(-0.651344180f, 0.758782387f)),
		std::make_pair(Vector2(631.0f, 467.0f), Vector2(0.978677809f, 0.205401510f))
	};

	std::vector<float> cutSizeLrg(sampleLocations.size(),160.0f);
	std::vector<float> cutSizeMed(sampleLocations.size(), 80.0f);
	std::vector<float> cutSizeSml(sampleLocations.size(), 40.0f);

	auto cutSamplesLrg = CreateSamples(sampleLocations, cutSizeLrg, L"large");	// large-sized cuts
	auto cutSamplesMed = CreateSamples(sampleLocations, cutSizeMed, L"large");	// medium-sized cuts (50% of large)
	auto cutSamplesSml = CreateSamples(sampleLocations, cutSizeSml, L"small");	// small-sized cuts (25% of large)

	RunTest(cutter, cutSamplesLrg, resolution, window, proj, view);
	RunTest(cutter, cutSamplesMed, resolution, window, proj, view);
	RunTest(cutter, cutSamplesSml, resolution, window, proj, view);
}
