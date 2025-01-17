/***************************************************************************
 * Copyright 1998-2017 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/mutex.hpp>

#include "luxrays/core/intersectiondevice.h"
#include "luxrays/core/virtualdevice.h"
#include "slg/slg.h"
#include "slg/engines/tilerepository.h"
#include "slg/engines/cpurenderengine.h"
#include "slg/engines/oclrenderengine.h"
#include "slg/engines/tilepathocl/tilepathocl.h"
#include "slg/engines/rtpathocl/rtpathocl.h"
#include "luxcore/luxcore.h"
#include "luxcore/luxcoreimpl.h"

using namespace std;
using namespace luxrays;
using namespace luxcore;
using namespace luxcore::detail;

//------------------------------------------------------------------------------
// FilmImpl
//------------------------------------------------------------------------------

FilmImpl::FilmImpl(const RenderSessionImpl &session) : renderSession(&session),
		standAloneFilm(NULL) {
}

FilmImpl::FilmImpl(const std::string &fileName) : renderSession(NULL) {
	standAloneFilm = slg::Film::LoadSerialized(fileName);
}

FilmImpl::~FilmImpl() {
	delete standAloneFilm;
}

slg::Film *FilmImpl::GetSLGFilm() const {
	if (renderSession)
		return renderSession->renderSession->film;
	else
		return standAloneFilm;
}

unsigned int FilmImpl::GetWidth() const {
	return GetSLGFilm()->GetWidth();
}

unsigned int FilmImpl::GetHeight() const {
	return GetSLGFilm()->GetHeight();
}

void FilmImpl::SaveOutputs() const {
	if (renderSession)
		renderSession->renderSession->SaveFilmOutputs();
	else
		throw runtime_error("Film::SaveOutputs() can not be used with a stand alone Film");
}

void FilmImpl::SaveOutput(const std::string &fileName, const FilmOutputType type, const Properties &props) const {
	GetSLGFilm()->Output(fileName, (slg::FilmOutputs::FilmOutputType)type, &props);
}

void FilmImpl::SaveFilm(const string &fileName) const {
	if (renderSession)
		renderSession->renderSession->SaveFilm(fileName);
	else
		slg::Film::SaveSerialized(fileName, standAloneFilm);
}

double FilmImpl::GetTotalSampleCount() const {
	return GetSLGFilm()->GetTotalSampleCount(); 
}

bool FilmImpl::HasOutput(const FilmOutputType type) const {
	return GetSLGFilm()->HasOutput((slg::FilmOutputs::FilmOutputType)type);
}

size_t FilmImpl::GetOutputSize(const FilmOutputType type) const {
	return GetSLGFilm()->GetOutputSize((slg::FilmOutputs::FilmOutputType)type);
}

unsigned int FilmImpl::GetRadianceGroupCount() const {
	return GetSLGFilm()->GetRadianceGroupCount();
}

void FilmImpl::GetOutputFloat(const FilmOutputType type, float *buffer, const unsigned int index) {
	if (renderSession) {
		boost::unique_lock<boost::mutex> lock(renderSession->renderSession->filmMutex);

		renderSession->renderSession->film->GetOutput<float>((slg::FilmOutputs::FilmOutputType)type, buffer, index);
	} else
		standAloneFilm->GetOutput<float>((slg::FilmOutputs::FilmOutputType)type, buffer, index);
}

void FilmImpl::GetOutputUInt(const FilmOutputType type, unsigned int *buffer, const unsigned int index) {
	if (renderSession) {
		boost::unique_lock<boost::mutex> lock(renderSession->renderSession->filmMutex);

		renderSession->renderSession->film->GetOutput<u_int>((slg::FilmOutputs::FilmOutputType)type, buffer, index);
	} else
		standAloneFilm->GetOutput<unsigned int>((slg::FilmOutputs::FilmOutputType)type, buffer, index);
}

unsigned int FilmImpl::GetChannelCount(const FilmChannelType type) const {
	return GetSLGFilm()->GetChannelCount((slg::Film::FilmChannelType)type);
}

const float *FilmImpl::GetChannelFloat(const FilmChannelType type, const unsigned int index) {
	if (renderSession) {
		boost::unique_lock<boost::mutex> lock(renderSession->renderSession->filmMutex);

		return renderSession->renderSession->film->GetChannel<float>((slg::Film::FilmChannelType)type, index);
	} else
		return standAloneFilm->GetChannel<float>((slg::Film::FilmChannelType)type, index);
}

const unsigned int *FilmImpl::GetChannelUInt(const FilmChannelType type, const unsigned int index) {
	if (renderSession) {
		boost::unique_lock<boost::mutex> lock(renderSession->renderSession->filmMutex);

		return renderSession->renderSession->film->GetChannel<unsigned int>((slg::Film::FilmChannelType)type, index);
	} else
		return standAloneFilm->GetChannel<unsigned int>((slg::Film::FilmChannelType)type, index);
}

void FilmImpl::Parse(const luxrays::Properties &props) {
	if (renderSession)
		throw runtime_error("Film::Parse() can be used only with a stand alone Film");
	else
		standAloneFilm->Parse(props);
}

//------------------------------------------------------------------------------
// CameraImpl
//------------------------------------------------------------------------------

CameraImpl::CameraImpl(const SceneImpl &scn) : scene(scn) {
}

CameraImpl::~CameraImpl() {
}

const CameraImpl::CameraType CameraImpl::GetType() const {
	return (Camera::CameraType)scene.scene->camera->GetType();
}

void CameraImpl::Translate(const float x, const float y, const float z) const {
	scene.scene->camera->Translate(Vector(x, y, z));
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::TranslateLeft(const float t) const {
	scene.scene->camera->TranslateLeft(t);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::TranslateRight(const float t) const {
	scene.scene->camera->TranslateRight(t);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::TranslateForward(const float t) const {
	scene.scene->camera->TranslateForward(t);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::TranslateBackward(const float t) const {
	scene.scene->camera->TranslateBackward(t);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::Rotate(const float angle, const float x, const float y, const float z) const {
	scene.scene->camera->Rotate(angle, Vector(x, y, z));
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::RotateLeft(const float angle) const {
	scene.scene->camera->RotateLeft(angle);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::RotateRight(const float angle) const {
	scene.scene->camera->RotateRight(angle);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::RotateUp(const float angle) const {
	scene.scene->camera->RotateUp(angle);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

void CameraImpl::RotateDown(const float angle) const {
	scene.scene->camera->RotateDown(angle);
	scene.scene->editActions.AddAction(slg::CAMERA_EDIT);
}

//------------------------------------------------------------------------------
// SceneImpl
//------------------------------------------------------------------------------

SceneImpl::SceneImpl(const float imageScale) {
	camera = new CameraImpl(*this);
	scene = new slg::Scene(imageScale);
	allocatedScene = true;
}

SceneImpl::SceneImpl(const string &fileName, const float imageScale) {
	camera = new CameraImpl(*this);
	scene = new slg::Scene(fileName, imageScale);
	allocatedScene = true;
}

SceneImpl::SceneImpl(slg::Scene *scn) {
	camera = new CameraImpl(*this);
	scene = scn;
	allocatedScene = false;
}

SceneImpl::~SceneImpl() {
	if (allocatedScene)
		delete scene;
	delete camera;
}

void SceneImpl::GetBBox(float min[3], float max[3]) const {
	const BBox &worldBBox = scene->dataSet->GetBBox();

	min[0] = worldBBox.pMin.x;
	min[1] = worldBBox.pMin.y;
	min[2] = worldBBox.pMin.z;

	max[0] = worldBBox.pMax.x;
	max[1] = worldBBox.pMax.y;
	max[2] = worldBBox.pMax.z;
}

const Camera &SceneImpl::GetCamera() const {
	return *camera;
}

bool SceneImpl::IsImageMapDefined(const std::string &imgMapName) const {
	return scene->IsImageMapDefined(imgMapName);
}

void SceneImpl::SetDeleteMeshData(const bool v) {
	scene->extMeshCache.SetDeleteMeshData(v);
}

void SceneImpl::DefineMesh(const std::string &meshName,
		const long plyNbVerts, const long plyNbTris,
		float *p, unsigned int *vi, float *n, float *uv,
		float *cols, float *alphas) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->DefineMesh(meshName, plyNbVerts, plyNbTris, (Point *)p,
			(Triangle *)vi, (Normal *)n, (UV *)uv,
			(Spectrum *)cols, alphas);
}

void SceneImpl::SaveMesh(const string &meshName, const string &fileName) {
	const ExtMesh *mesh = scene->extMeshCache.GetExtMesh(meshName);
	mesh->WritePly(fileName);
}

void SceneImpl::DefineStrands(const string &shapeName, const cyHairFile &strandsFile,
		const StrandsTessellationType tesselType,
		const unsigned int adaptiveMaxDepth, const float adaptiveError,
		const unsigned int solidSideCount, const bool solidCapBottom, const bool solidCapTop,
		const bool useCameraPosition) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->DefineStrands(shapeName, strandsFile,
			(slg::StrendsShape::TessellationType)tesselType, adaptiveMaxDepth, adaptiveError,
			solidSideCount, solidCapBottom, solidCapTop,
			useCameraPosition);
}

bool SceneImpl::IsMeshDefined(const std::string &meshName) const {
	return scene->IsMeshDefined(meshName);
}

bool SceneImpl::IsTextureDefined(const std::string &texName) const {
	return scene->IsTextureDefined(texName);
}

bool SceneImpl::IsMaterialDefined(const std::string &matName) const {
	return scene->IsMaterialDefined(matName);
}

const unsigned int SceneImpl::GetLightCount() const {
	return scene->lightDefs.GetSize();
}

const unsigned int  SceneImpl::GetObjectCount() const {
	return scene->objDefs.GetSize();
}

void SceneImpl::Parse(const Properties &props) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->Parse(props);
}

void SceneImpl::UpdateObjectTransformation(const std::string &objName, const float *transMat) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	// I have to transpose the matrix
	const Matrix4x4 mat(
		transMat[0], transMat[4], transMat[8], transMat[12],
		transMat[1], transMat[5], transMat[9], transMat[13],
		transMat[2], transMat[6], transMat[10], transMat[14],
		transMat[3], transMat[7], transMat[11], transMat[15]);
	const Transform trans(mat);
	scene->UpdateObjectTransformation(objName, trans);
}

void SceneImpl::UpdateObjectMaterial(const std::string &objName, const std::string &matName) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->UpdateObjectMaterial(objName, matName);
}

void SceneImpl::DeleteObject(const string &objName) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->DeleteObject(objName);
}

void SceneImpl::DeleteLight(const string &lightName) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->DeleteLight(lightName);
}

void SceneImpl::RemoveUnusedImageMaps() {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->RemoveUnusedImageMaps();
}

void SceneImpl::RemoveUnusedTextures() {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->RemoveUnusedTextures();
}

void SceneImpl::RemoveUnusedMaterials() {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->RemoveUnusedMaterials();
}

void SceneImpl::RemoveUnusedMeshes() {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->RemoveUnusedMeshes();
}

const Properties &SceneImpl::ToProperties() const {
	if (!scenePropertiesCache.GetSize())
		scenePropertiesCache << scene->ToProperties();

	return scenePropertiesCache;
}

void SceneImpl::DefineImageMapUChar(const std::string &imgMapName,
		unsigned char *pixels, const float gamma, const unsigned int channels,
		const unsigned int width, const unsigned int height,
		ChannelSelectionType selectionType) {
	scene->DefineImageMap<u_char>(imgMapName, pixels, gamma, channels,
			width, height, (slg::ImageMapStorage::ChannelSelectionType)selectionType);
}

void SceneImpl::DefineImageMapHalf(const std::string &imgMapName,
		unsigned short *pixels, const float gamma, const unsigned int channels,
		const unsigned int width, const unsigned int height,
		ChannelSelectionType selectionType) {
	scene->DefineImageMap<half>(imgMapName, (half *)pixels, gamma, channels,
			width, height, (slg::ImageMapStorage::ChannelSelectionType)selectionType);
}

void SceneImpl::DefineImageMapFloat(const std::string &imgMapName,
		float *pixels, const float gamma, const unsigned int channels,
		const unsigned int width, const unsigned int height,
		ChannelSelectionType selectionType) {
	scene->DefineImageMap<float>(imgMapName, pixels, gamma, channels,
			width, height, (slg::ImageMapStorage::ChannelSelectionType)selectionType);
}

// Note: this method is not part of LuxCore API and it is used only internally
void SceneImpl::DefineMesh(const string &meshName, ExtTriangleMesh *mesh) {
	// Invalidate the scene properties cache
	scenePropertiesCache.Clear();

	scene->DefineMesh(meshName, mesh);
}

Point *SceneImpl::AllocVerticesBuffer(const unsigned int meshVertCount) {
	return TriangleMesh::AllocVerticesBuffer(meshVertCount);
}

Triangle *SceneImpl::AllocTrianglesBuffer(const unsigned int meshTriCount) {
	return TriangleMesh::AllocTrianglesBuffer(meshTriCount);
}

//------------------------------------------------------------------------------
// RenderConfigImpl
//------------------------------------------------------------------------------


RenderConfigImpl::RenderConfigImpl(const Properties &props, SceneImpl *scn) {
	if (scn) {
		scene = scn;
		allocatedScene = false;
		renderConfig = new slg::RenderConfig(props, scene->scene);
	} else {
		renderConfig = new slg::RenderConfig(props);
		scene = new SceneImpl(renderConfig->scene);
		allocatedScene = true;
	}
}

RenderConfigImpl::~RenderConfigImpl() {
	delete renderConfig;
	if (allocatedScene)
		delete scene;
}

const Properties &RenderConfigImpl::GetProperties() const {
	return renderConfig->cfg;
}

const Property RenderConfigImpl::GetProperty(const std::string &name) const {
	return renderConfig->GetProperty(name);
}

const Properties &RenderConfigImpl::ToProperties() const {
	return renderConfig->ToProperties();
}

Scene &RenderConfigImpl::GetScene() const {
	return *scene;
}

void RenderConfigImpl::Parse(const Properties &props) {
	renderConfig->Parse(props);
}

void RenderConfigImpl::Delete(const string &prefix) {
	renderConfig->Delete(prefix);
}

bool RenderConfigImpl::GetFilmSize(unsigned int *filmFullWidth, unsigned int *filmFullHeight,
		unsigned int *filmSubRegion) const {
	return renderConfig->GetFilmSize(filmFullWidth, filmFullHeight, filmSubRegion);
}

void RenderConfigImpl::DeleteSceneOnExit() {
	allocatedScene = true;
}

const Properties &RenderConfigImpl::GetDefaultProperties() {
	return slg::RenderConfig::GetDefaultProperties();
}

//------------------------------------------------------------------------------
// RenderStateImpl
//------------------------------------------------------------------------------

RenderStateImpl::RenderStateImpl(const std::string &fileName) {
	renderState = slg::RenderState::LoadSerialized(fileName);
}

RenderStateImpl::RenderStateImpl(slg::RenderState *state) {
	renderState = state;
}

RenderStateImpl::~RenderStateImpl() {
	delete renderState;
}

void RenderStateImpl::Save(const std::string &fileName) const {
	renderState->SaveSerialized(fileName);
}

//------------------------------------------------------------------------------
// RenderSessionImpl
//------------------------------------------------------------------------------

RenderSessionImpl::RenderSessionImpl(const RenderConfigImpl *config, RenderStateImpl *startState, FilmImpl *startFilm) :
		renderConfig(config) {
	film = new FilmImpl(*this);

	renderSession = new slg::RenderSession(config->renderConfig,
			startState ? startState->renderState : NULL,
			startFilm ? startFilm->standAloneFilm : NULL);

	if (startState) {
		// slg::RenderSession will take care of deleting startState->renderState
		startState->renderState = NULL;
		delete startState;
	}

	if (startFilm) {
		// slg::RenderSession will take care of deleting startFilm->standAloneFilm too
		startFilm->standAloneFilm = NULL;
		delete startFilm;
	}
}

RenderSessionImpl::RenderSessionImpl(const RenderConfigImpl *config, const std::string &startStateFileName,
		const std::string &startFilmFileName) :
		renderConfig(config) {
	film = new FilmImpl(*this);

	auto_ptr<slg::Film> startFilm(slg::Film::LoadSerialized(startFilmFileName));
	auto_ptr<slg::RenderState> startState(slg::RenderState::LoadSerialized(startStateFileName));

	renderSession = new slg::RenderSession(config->renderConfig,
			startState.release(), startFilm.release());
}

RenderSessionImpl::~RenderSessionImpl() {
	delete film;
	delete renderSession;
}
const RenderConfig &RenderSessionImpl::GetRenderConfig() const {
	return *renderConfig;
}

RenderState *RenderSessionImpl::GetRenderState() {
	return new RenderStateImpl(renderSession->GetRenderState());
}

void RenderSessionImpl::Start() {
	renderSession->Start();

	// In order to populate the stats.* Properties
	UpdateStats();
}

void RenderSessionImpl::Stop() {
	renderSession->Stop();
}

bool RenderSessionImpl::IsStarted() const {
	return renderSession->IsStarted();
}

void RenderSessionImpl::BeginSceneEdit() {
	renderSession->BeginSceneEdit();
}

void RenderSessionImpl::EndSceneEdit() {
	renderSession->EndSceneEdit();
}

bool RenderSessionImpl::IsInSceneEdit() const {
	return renderSession->IsInSceneEdit();
}

void RenderSessionImpl::Pause() {
	renderSession->Pause();
}

void RenderSessionImpl::Resume() {
	renderSession->Resume();
}

bool RenderSessionImpl::IsInPause() const {
	return renderSession->IsInPause();
}

bool RenderSessionImpl::HasDone() const {
	return renderSession->renderEngine->HasDone();
}

void RenderSessionImpl::WaitForDone() const {
	renderSession->renderEngine->WaitForDone();
}

void RenderSessionImpl::WaitNewFrame() {
	renderSession->renderEngine->WaitNewFrame();
}

bool RenderSessionImpl::NeedPeriodicFilmSave() {
	return renderSession->NeedPeriodicFilmSave();
}

Film &RenderSessionImpl::GetFilm() {
	return *film;
}

static void SetTileProperties(Properties &props, const string &prefix,
		const deque<const slg::TileRepository::Tile *> &tiles) {
	props.Set(Property(prefix + ".count")((unsigned int)tiles.size()));
	Property tileCoordProp(prefix + ".coords");
	Property tilePassProp(prefix + ".pass");
	Property tileErrorProp(prefix + ".error");

	BOOST_FOREACH(const slg::TileRepository::Tile *tile, tiles) {
		tileCoordProp.Add(tile->xStart).Add(tile->yStart);
		tilePassProp.Add(tile->pass);
		tileErrorProp.Add(tile->error);
	}

	props.Set(tileCoordProp);
	props.Set(tilePassProp);
	props.Set(tileErrorProp);
}

void RenderSessionImpl::UpdateStats() {
	// Film update may be required by some render engine to
	// update statistics, convergence test and more
	renderSession->renderEngine->UpdateFilm();

	stats.Set(Property("stats.renderengine.total.raysec")(renderSession->renderEngine->GetTotalRaysSec()));
	stats.Set(Property("stats.renderengine.total.samplesec")(renderSession->renderEngine->GetTotalSamplesSec()));
	stats.Set(Property("stats.renderengine.total.samplecount")(renderSession->renderEngine->GetTotalSampleCount()));
	stats.Set(Property("stats.renderengine.pass")(renderSession->renderEngine->GetPass()));
	stats.Set(Property("stats.renderengine.time")(renderSession->renderEngine->GetRenderingTime()));
	stats.Set(Property("stats.renderengine.convergence")(renderSession->renderEngine->GetConvergence()));
	
	// Intersection devices statistics
	const vector<IntersectionDevice *> &idevices = renderSession->renderEngine->GetIntersectionDevices();

	// Replace all virtual devices with real
	vector<IntersectionDevice *> realDevices;
	for (size_t i = 0; i < idevices.size(); ++i) {
		VirtualIntersectionDevice *vdev = dynamic_cast<VirtualIntersectionDevice *>(idevices[i]);
		if (vdev) {
			const vector<IntersectionDevice *> &realDevs = vdev->GetRealDevices();
			realDevices.insert(realDevices.end(), realDevs.begin(), realDevs.end());
		} else
			realDevices.push_back(idevices[i]);
	}

	boost::unordered_map<string, unsigned int> devCounters;
	Property devicesNames("stats.renderengine.devices");
	double totalPerf = 0.0;
	BOOST_FOREACH(IntersectionDevice *dev, realDevices) {
		const string &devName = dev->GetName();

		// Append a device index for the case where the same device is used multiple times
		unsigned int index = devCounters[devName]++;
		const string uniqueName = devName + "-" + ToString(index);
		devicesNames.Add(uniqueName);

		const string prefix = "stats.renderengine.devices." + uniqueName;
		totalPerf += dev->GetTotalPerformance();
		stats.Set(Property(prefix + ".performance.total")(dev->GetTotalPerformance()));
		stats.Set(Property(prefix + ".performance.serial")(dev->GetSerialPerformance()));
		stats.Set(Property(prefix + ".performance.dataparallel")(dev->GetDataParallelPerformance()));
		stats.Set(Property(prefix + ".memory.total")((u_longlong)dev->GetMaxMemory()));
		stats.Set(Property(prefix + ".memory.used")((u_longlong)dev->GetUsedMemory()));
	}
	stats.Set(devicesNames);
	stats.Set(Property("stats.renderengine.performance.total")(totalPerf));

	// The explicit cast to size_t is required by VisualC++
	stats.Set(Property("stats.dataset.trianglecount")(renderSession->renderConfig->scene->dataSet->GetTotalTriangleCount()));

	// Some engine specific statistic
	switch (renderSession->renderEngine->GetType()) {
#if !defined(LUXRAYS_DISABLE_OPENCL)
		case slg::RTPATHOCL: {
			slg::RTPathOCLRenderEngine *engine = (slg::RTPathOCLRenderEngine *)renderSession->renderEngine;
			stats.Set(Property("stats.rtpathocl.frame.time")(engine->GetFrameTime()));
			break;
		}
		case slg::TILEPATHOCL: {
			slg::TilePathOCLRenderEngine *engine = (slg::TilePathOCLRenderEngine *)renderSession->renderEngine;
			
			stats.Set(Property("stats.tilepath.tiles.size.x")(engine->GetTileWidth()));
			stats.Set(Property("stats.tilepath.tiles.size.y")(engine->GetTileHeight()));

			// Pending tiles
			{
				deque<const slg::TileRepository::Tile *> tiles;
				engine->GetPendingTiles(tiles);
				SetTileProperties(stats, "stats.tilepath.tiles.pending", tiles);
			}

			// Not converged tiles
			{
				deque<const slg::TileRepository::Tile *> tiles;
				engine->GetNotConvergedTiles(tiles);
				SetTileProperties(stats, "stats.tilepath.tiles.notconverged", tiles);
			}

			// Converged tiles
			{
				deque<const slg::TileRepository::Tile *> tiles;
				engine->GetConvergedTiles(tiles);
				SetTileProperties(stats, "stats.tilepath.tiles.converged", tiles);
			}
			break;
		}
#endif
		case slg::TILEPATHCPU: {
			slg::CPUTileRenderEngine *engine = (slg::CPUTileRenderEngine *)renderSession->renderEngine;

			stats.Set(Property("stats.tilepath.tiles.size.x")(engine->GetTileWidth()));
			stats.Set(Property("stats.tilepath.tiles.size.y")(engine->GetTileHeight()));

			// Pending tiles
			{
				deque<const slg::TileRepository::Tile *> tiles;
				engine->GetPendingTiles(tiles);
				SetTileProperties(stats, "stats.tilepath.tiles.pending", tiles);
			}

			// Not converged tiles
			{
				deque<const slg::TileRepository::Tile *> tiles;
				engine->GetNotConvergedTiles(tiles);
				SetTileProperties(stats, "stats.tilepath.tiles.notconverged", tiles);
			}

			// Converged tiles
			{
				deque<const slg::TileRepository::Tile *> tiles;
				engine->GetConvergedTiles(tiles);
				SetTileProperties(stats, "stats.tilepath.tiles.converged", tiles);
			}
			break;
		}
		default:
			break;
	}
}

const Properties &RenderSessionImpl::GetStats() const {
	return stats;
}

void RenderSessionImpl::Parse(const Properties &props) {
	renderSession->Parse(props);
}
