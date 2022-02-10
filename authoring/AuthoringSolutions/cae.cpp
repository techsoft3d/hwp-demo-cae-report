
#include <string>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <fstream>

#include "sc_store.h"
#include "sc_assemblytree.h"
#include "tc_io.h"
#include "hoops_license.h"
#include "toUtilities.h"

class Logger : public SC::Store::Logger {
public:
	virtual void Message(const char * message) const {
		printf("%s\n", message);
	}
};

struct stCoordinate
{
	float x;
	float y;
	float z;
};

struct stNodeData
{
	int iNodeID;
	float fMises;
	float fTresca;
	float fSigma1;
	float fSigma2;
	float fSigma3;
	float fStrain;
};

struct analysis_data
{
	analysis_data ():
		iMaxMisesNode(0.0),
		fMinMises(100000.0),
		fMaxTresca(0.0),
		fMaxPrinciple(0.0),
		fMidPrinciple(0.0),
		fMinPrinciple(0.0),
		fMaxPrincipleStrain(0.0)
	{}
	int iMaxMisesNode;
	int iMaxMisesID;
	int iMaxTrescaNode;
	int iMaxTrescaID;
	int iPrincipleStressNode;
	int iPrincipleStressID;
	int iMaxPrincipleStrainNode;
	int iMaxPrincipleStrainID;

	float fMinMises;
	float fMaxMises;
	float fMaxTresca;
	float fMaxPrinciple;
	float fMidPrinciple;
	float fMinPrinciple;
	float fMaxPrincipleStrain;

	stCoordinate stMaxMisesCoord;
	stCoordinate stMaxTrescaCoord;
	stCoordinate stPrincipleStressCoord;
	stCoordinate stMaxPrincipleStrainCoord;

	std::vector<stNodeData> stAllNodeData;
};

void LoadResultDataFile(const char* filePath, std::vector<double> &misesData, analysis_data &analysisData)
{
	char szDrive[256], szPath[256], szFName[256], szExt[256], szDir[256];
	_splitpath_s(filePath, szDrive, sizeof(szDrive), szPath, sizeof(szPath), szFName, sizeof(szFName), szExt, sizeof(szExt));

	FILE* fp;
	fopen_s(&fp, filePath, "r");
	if (fp == NULL) {
		return;
	}

	int nFlag = 0;
	char szBuffer[2048] = { '\0' };
	int iID = 0;

	while (fgets(szBuffer, 2048, fp) > 0) {
		std::vector<std::string> sTexts = split_str(szBuffer);
		if (sTexts.empty()) {
			continue;
		}

		if (strncmp(sTexts[0].c_str(), "POINT", 5) == 0) {
			nFlag = 1;
			continue;
		}

		if (nFlag == 1 && sTexts.size() == 7) {
			stNodeData nodeData;
			int iNode = stoi(sTexts[0].c_str());
			nodeData.iNodeID = iNode;
			// Mises
			float fMises = strtof(sTexts[1].c_str(), NULL);
			nodeData.fMises = fMises;
			if (fMises < analysisData.fMinMises) {
				analysisData.fMinMises = fMises;
			}
			if (analysisData.fMaxMises < fMises) {
				analysisData.fMaxMises = fMises;
				analysisData.iMaxMisesNode = iNode;
				analysisData.iMaxMisesID = iID;
			}
			misesData.push_back(fMises);

			// Tresca
			float fTrasca = strtof(sTexts[5].c_str(), NULL);
			nodeData.fTresca = fTrasca;
			if (analysisData.fMaxTresca < fTrasca) {
				analysisData.fMaxTresca = fTrasca;
				analysisData.iMaxTrescaNode = iNode;
				analysisData.iMaxTrescaID = iID;
			}

			// Principle
			float fPrinciple = strtof(sTexts[2].c_str(), NULL);
			nodeData.fSigma1 = fPrinciple;
			nodeData.fSigma2 = strtof(sTexts[3].c_str(), NULL);
			nodeData.fSigma3 = strtof(sTexts[4].c_str(), NULL);
			if (analysisData.fMaxPrinciple < fPrinciple) {
				analysisData.fMaxPrinciple = fPrinciple;
				analysisData.fMidPrinciple = strtof(sTexts[3].c_str(), NULL);
				analysisData.fMinPrinciple = strtof(sTexts[4].c_str(), NULL);
				analysisData.iPrincipleStressNode = iNode;
				analysisData.iPrincipleStressID = iID;
			}

			// Strain
			float fStrain = strtof(sTexts[6].c_str(), NULL);
			nodeData.fStrain = fStrain;
			if (analysisData.fMaxPrincipleStrain < fStrain) {
				analysisData.fMaxPrincipleStrain = fStrain;
				analysisData.iMaxPrincipleStrainNode = iNode;
				analysisData.iMaxPrincipleStrainID = iID;
			}
			analysisData.stAllNodeData.push_back(nodeData);
			iID++;
		}
	}
	fclose(fp);
}

void LoadTessDataFile(const char* filePath, std::vector<float> &pointData, std::vector<int> &faceData)
{
	char szDrive[256], szPath[256], szFName[256], szExt[256], szDir[256];
	_splitpath_s(filePath, szDrive, sizeof(szDrive), szPath, sizeof(szPath), szFName, sizeof(szFName), szExt, sizeof(szExt));

	FILE* fp;
	fopen_s(&fp, filePath, "r");
	if (fp == NULL) {
		return;
	}

	int nFlag = 0;
	char szBuffer[2048] = { '\0' };
	int iPointCnt = 0;
	std::unordered_map<int, int> pointMap;

	while (fgets(szBuffer, 2048, fp) > 0) {
		std::vector<std::string> sTexts = split_str(szBuffer);
		if (sTexts.empty()) {
			continue;
		}

		if (strncmp(sTexts[0].c_str(), "POINT", 5) == 0) {
			nFlag = 1;
			continue;
		} else if (strncmp(sTexts[0].c_str(), "ELEMENT", 7) == 0) {
			nFlag = 2;
			continue;
		}

		if (nFlag == 1 && sTexts.size() == 4) {
			int nID = atoi(sTexts[0].c_str());
			for (int i = 0; i < 3; i++) {
				pointData.push_back(strtof(sTexts[i + 1].c_str(), NULL));
			}

			pointMap[nID] = iPointCnt++;

		} else if (nFlag == 2 && sTexts.size() >= 5){
			for (int i = 0; i < 3; i++) {
				int iP = atoi(sTexts[i + 2].c_str());
				faceData.push_back(pointMap[iP]);
			}
		}
	}
	fclose(fp);
}

void ExportFile(const string filePath, analysis_data analysisData)
{
	std::ofstream ofs(filePath);
	ofs << "{" << endl;
	// header
	ofs << "\t\"header\":{" << endl;
	ofs << "\t\t\"date\":\"July / 13 / 2017\", \"number\":\"TS11 - 231\", \"author\":\"Toshiaki Kawabata\"" << endl;
	ofs << "\t}," << endl;

	// results
	ofs << "\t\"results\":\{" << endl;
	ofs << "\t\t\"mises\":{\"node\":" << analysisData.iMaxMisesNode << ", \"stress\":" << analysisData.fMaxMises << ", \"min\":" << analysisData.fMinMises << "}," << endl;
	ofs << "\t\t\"tresca\":{\"node\":" << analysisData.iMaxTrescaNode << ", \"stress\":" << analysisData.fMaxTresca << "}," << endl;
	ofs << "\t\t\"principleStress\":{\"signam1\":{\"node\":" << analysisData.iPrincipleStressNode << ", \"stress\":" << analysisData.fMaxPrinciple << 
		"}, \"signam2\":{\"node\":" << analysisData.iPrincipleStressNode << ", \"stress\":" << analysisData.fMidPrinciple << 
		"}, \"signam3\":{\"node\":" << analysisData.iPrincipleStressNode << ", \"stress\":" << analysisData.fMinPrinciple << "}}, " << endl;
	ofs << "\t\t\"principleStrain\":{\"node\":" << analysisData.iMaxPrincipleStrainNode << ", \"strain\":" << analysisData.fMaxPrincipleStrain << "}" << endl;
	ofs << "\t}," << endl;

	// annotations
	ofs << "\t\"annotations\":\{" << endl;
	ofs << "\t\t\"mises\":{" << endl;
	ofs << "\t\t\t\"annotation\":{\"label\":\"Max Von Mises stress: " << analysisData.fMaxMises << " MPa\", \"leaderAnchor\":{\"x\":" << analysisData.stMaxMisesCoord.x << ",\"y\":" << analysisData.stMaxMisesCoord.y << ",\"z\":" << analysisData.stMaxMisesCoord.z <<
		"},\"textBoxAnchor\":{\"x\":" << analysisData.stMaxMisesCoord.x << ",\"y\":" << analysisData.stMaxMisesCoord.y << ",\"z\":" << analysisData.stMaxMisesCoord.z << "}}" << endl;
	ofs << "\t\t}," << endl;

	ofs << "\t\t\"tresca\":{" << endl;
	ofs << "\t\t\t\"annotation\":{\"label\":\"Max Von Tresca stress: " << analysisData.fMaxTresca << " MPa\", \"leaderAnchor\":{\"x\":" << analysisData.stMaxTrescaCoord.x << ",\"y\":" << analysisData.stMaxTrescaCoord.y << ",\"z\":" << analysisData.stMaxTrescaCoord.z <<
		"},\"textBoxAnchor\":{\"x\":" << analysisData.stMaxTrescaCoord.x << ",\"y\":" << analysisData.stMaxTrescaCoord.y << ",\"z\":" << analysisData.stMaxTrescaCoord.z << "}}" << endl;
	ofs << "\t\t}," << endl;

	ofs << "\t\t\"sigma1\":{" << endl;
	ofs << "\t\t\t\"annotation\":{\"label\":\"Max Principle stress Sigma1: " << analysisData.fMaxPrinciple << " MPa\", \"leaderAnchor\":{\"x\":" << analysisData.stPrincipleStressCoord.x << ",\"y\":" << analysisData.stPrincipleStressCoord.y << ",\"z\":" << analysisData.stPrincipleStressCoord.z <<
		"},\"textBoxAnchor\":{\"x\":" << analysisData.stPrincipleStressCoord.x << ",\"y\":" << analysisData.stPrincipleStressCoord.y << ",\"z\":" << analysisData.stPrincipleStressCoord.z << "}}" << endl;
	ofs << "\t\t}," << endl;

	ofs << "\t\t\"sigma2\":{" << endl;
	ofs << "\t\t\t\"annotation\":{\"label\":\"Max Principle stress Sigma2: " << analysisData.fMidPrinciple << " MPa\", \"leaderAnchor\":{\"x\":" << analysisData.stPrincipleStressCoord.x << ",\"y\":" << analysisData.stPrincipleStressCoord.y << ",\"z\":" << analysisData.stPrincipleStressCoord.z <<
		"},\"textBoxAnchor\":{\"x\":" << analysisData.stPrincipleStressCoord.x << ",\"y\":" << analysisData.stPrincipleStressCoord.y << ",\"z\":" << analysisData.stPrincipleStressCoord.z << "}}" << endl;
	ofs << "\t\t}," << endl;

	ofs << "\t\t\"sigma3\":{" << endl;
	ofs << "\t\t\t\"annotation\":{\"label\":\"Max Principle stress Sigma3: " << analysisData.fMinPrinciple << " MPa\", \"leaderAnchor\":{\"x\":" << analysisData.stPrincipleStressCoord.x << ",\"y\":" << analysisData.stPrincipleStressCoord.y << ",\"z\":" << analysisData.stPrincipleStressCoord.z <<
		"},\"textBoxAnchor\":{\"x\":" << analysisData.stPrincipleStressCoord.x << ",\"y\":" << analysisData.stPrincipleStressCoord.y << ",\"z\":" << analysisData.stPrincipleStressCoord.z << "}}" << endl;
	ofs << "\t\t}," << endl;

	ofs << "\t\t\"strain\":{" << endl;
	ofs << "\t\t\t\"annotation\":{\"label\":\"Max Principle strain: " << analysisData.fMaxPrincipleStrain << " mm\", \"leaderAnchor\":{\"x\":" << analysisData.stMaxPrincipleStrainCoord.x << ",\"y\":" << analysisData.stMaxPrincipleStrainCoord.y << ",\"z\":" << analysisData.stMaxPrincipleStrainCoord.z <<
		"},\"textBoxAnchor\":{\"x\":" << analysisData.stMaxPrincipleStrainCoord.x << ",\"y\":" << analysisData.stMaxPrincipleStrainCoord.y << ",\"z\":" << analysisData.stMaxPrincipleStrainCoord.z << "}}" << endl;
	ofs << "\t\t}" << endl;
	ofs << "\t}," << endl;

	// node data
	ofs << "\t\"nodes\":" << endl;
	ofs << "\t[" << endl;

	for (int i = 0; i < analysisData.stAllNodeData.size(); i++) {
		stNodeData node = analysisData.stAllNodeData[i];
		ofs << "\t\t{\"id\":" << node.iNodeID <<
			",\"mises\":" << node.fMises <<
			",\"tresca\":" << node.fTresca <<
			",\"sigma1\":" << node.fSigma1 <<
			",\"sigma2\":" << node.fSigma2 <<
			",\"sigma3\":" << node.fSigma3 <<
			",\"strain\":" << node.fStrain;
		if (i < analysisData.stAllNodeData.size() - 1)
			ofs << "}," << endl;
		else
			ofs << "}" << endl;
	}

	ofs << "\t]" << endl;

	ofs << "}" << endl;
}

int main(int argc, char ** argv)
{
	if (argc != 2) {
		printf("Need to supply a path to cache directory.\n");
		return 1;
	}
	
	std::string cache_path = argv[1];
	std::string model_name = "con-rod";
	Logger logger;

	try {
		// read CAE data
		std::vector<float> pointData;
		std::vector<int> facetData;
		LoadTessDataFile("data\\conrod.tessdat.csv", pointData, facetData);

		std::vector<double> misesData;
		float fMisesMin = 100000, fMisesMax = -100000;
		analysis_data analysisData;
		LoadResultDataFile("data\\conrod.result.csv", misesData, analysisData);

		fMisesMin = analysisData.fMinMises;
		fMisesMax = analysisData.fMaxMises;

		int iID = analysisData.iMaxMisesID;
		analysisData.stMaxMisesCoord.x = pointData[3 * iID];
		analysisData.stMaxMisesCoord.y = pointData[3 * iID + 1];
		analysisData.stMaxMisesCoord.z = pointData[3 * iID + 2];

		iID = analysisData.iMaxTrescaID;
		analysisData.stMaxTrescaCoord.x = pointData[3 * iID];
		analysisData.stMaxTrescaCoord.y = pointData[3 * iID + 1];
		analysisData.stMaxTrescaCoord.z = pointData[3 * iID + 2];

		iID = analysisData.iPrincipleStressID;
		analysisData.stPrincipleStressCoord.x = pointData[3 * iID];
		analysisData.stPrincipleStressCoord.y = pointData[3 * iID + 1];
		analysisData.stPrincipleStressCoord.z = pointData[3 * iID + 2];

		iID = analysisData.iMaxPrincipleStrainID;
		analysisData.stMaxPrincipleStrainCoord.x = pointData[3 * iID];
		analysisData.stMaxPrincipleStrainCoord.y = pointData[3 * iID + 1];
		analysisData.stMaxPrincipleStrainCoord.z = pointData[3 * iID + 2];

		ExportFile("C:\\temp\\" + model_name + ".json", analysisData);

		int iPointCnt = pointData.size() / 3;
		int iFaceCnt = facetData.size() / 3;

		// Open a read/write view of the cache.
		// The directory must exist beforehand. If not, this throws an exception.
		SC::Store::Database::SetLicense(HOOPS_LICENSE);
		SC::Store::Cache cache = SC::Store::Database::Open(logger);

		std::string file_path_string = cache_path;
		file_path_string += "/";
		file_path_string += model_name;

		// Does the model in question exist?
		if (cache.Exists((cache_path + "\\" + model_name).c_str())) {
			TC::IO::remove_all(TC::IO::path(TC::IO::utf8_to_path((cache_path + "\\" + model_name).c_str())));
		}


		// Create and open the model we care about.
		SC::Store::Model model = cache.Open(file_path_string.c_str());

		SC::Store::InclusionKey model_inclusion_key = model.Include(model);

		SC::Store::Mesh mesh;
		
		mesh.flags = (SC::Store::Mesh::Bits)(
			SC::Store::Mesh::CounterClockwiseWinding |
			SC::Store::Mesh::TwoSided | 
			SC::Store::Mesh::FaceNormals | 
			SC::Store::Mesh::FaceUVs | 
			SC::Store::Mesh::FaceRGBA32s |
			SC::Store::Mesh::LineRGBA32s |
			SC::Store::Mesh::PointRGBA32s
			);

		mesh.point_count = iPointCnt;
		mesh.points = (SC::Store::Point const *)&pointData[0];
		// (Alternatively a SC::Store::Point could be made directly instead of being cast from a float array.)

		std::vector<float>normalData;
		for (int i = 0; i < iFaceCnt; i++) {
			double dVerts[9];
			double dN[3] = { 0.0 };
			for (int j = 0; j < 3; j++) {
				int iPointId = facetData[3 * i + j];
				dVerts[3 * j] = pointData[3 * iPointId];
				dVerts[3 * j + 1] = pointData[3 * iPointId + 1];
				dVerts[3 * j + 2] = pointData[3 * iPointId + 2];
			}
			CalcNormal(&dVerts[0], &dVerts[3], &dVerts[6], dN);
			normalData.push_back(dN[0]);
			normalData.push_back(dN[1]);
			normalData.push_back(dN[2]);
		}

		mesh.normal_count = iFaceCnt;
		mesh.normals = (SC::Store::Normal const *)&normalData[0];

		float uv_data[] = {
			0.0f, 0.0f, // UV parameter 0
			1.0f, 0.0f, // UV parameter 1
			0.0f, 1.0f, // UV parameter 2
		};
		mesh.uv_count = 3;
		mesh.uvs = (SC::Store::UV const *)uv_data;

		// Each channel in an RGBA32 is an unsigned byte (with values in the inclusive range [0, 255]).
		SC::Store::RGBA32 rgba32[] = {
			SC::Store::RGBA32(0xFF, 0x00, 0x00, 0xFF), // Color 0 (Red)
			SC::Store::RGBA32(0x00, 0xFF, 0x00, 0xFF), // Color 1 (Green)
			SC::Store::RGBA32(0x00, 0x00, 0xFF, 0xFF), // Color 2 (Blue)
		};

		std::vector<SC::Store::RGBA32>rgba;
		for (int i = 0; i < iPointCnt; i++) {
			double dCol[3] = { 0.0 };
			CalcColor(misesData[i], fMisesMin, fMisesMax, dCol);
			uint8_t r = 255 * dCol[0];
			uint8_t g = 255 * dCol[1];
			uint8_t b = 255 * dCol[2];
			rgba.push_back(SC::Store::RGBA32(r, g, b, 0xFF));
		}

		mesh.rgba32_count = iPointCnt;
		mesh.rgba32s = &rgba[0];
		

		// Add a single face that consists of a single triangle.
		mesh.face_elements.resize(iFaceCnt);
		mesh.polyline_elements.resize(iFaceCnt);
		for (int i = 0; i < iFaceCnt; i++) {
			SC::Store::MeshElement & face_mesh_element = mesh.face_elements[i];
			// Indices for vertex 0
			face_mesh_element.indices.push_back(facetData[3 * i]); // Point 0
			face_mesh_element.indices.push_back(i); // Normal 0
			face_mesh_element.indices.push_back(0); // UV parameter 0
			face_mesh_element.indices.push_back(facetData[3 * i]); // Color 0

			// Indices for vertex 1
			face_mesh_element.indices.push_back(facetData[3 * i + 1]); // Point 1
			face_mesh_element.indices.push_back(i); // Normal 1
			face_mesh_element.indices.push_back(1); // UV parameter 1
			face_mesh_element.indices.push_back(facetData[3 * i + 1]); // Color 1

			// Indices for vertex 2
			face_mesh_element.indices.push_back(facetData[3 * i + 2]); // Point 2
			face_mesh_element.indices.push_back(i); // Normal 2
			face_mesh_element.indices.push_back(2); // UV parameter 2
			face_mesh_element.indices.push_back(facetData[3 * i + 2]); // Color 2

			SC::Store::MeshElement & edges = mesh.polyline_elements[i];
			edges.indices.push_back(facetData[3 * i]);
			edges.indices.push_back(facetData[3 * i]);
			edges.indices.push_back(facetData[3 * i + 1]);
			edges.indices.push_back(facetData[3 * i + 1]);
			edges.indices.push_back(facetData[3 * i + 2]);
			edges.indices.push_back(facetData[3 * i + 2]);
			edges.indices.push_back(facetData[3 * i]);
			edges.indices.push_back(facetData[3 * i]);
		}

		mesh.point_elements.resize(iPointCnt);
		for (int i = 0; i < iPointCnt; i++) {
			SC::Store::MeshElement & point = mesh.point_elements[i];
			point.indices.push_back(i);
			point.indices.push_back(i);
		}

		SC::Store::MeshKey mesh_key = model.Insert(mesh);

		SC::Store::Color color(0, 0, 0, 1);
		SC::Store::MaterialKey material_key = model.Insert(color);

		SC::Store::InstanceKey instance_key = model.Instance(mesh_key, SC::Store::MatrixKey(), material_key, material_key, material_key);

		SC::Store::AssemblyTree assembly_tree(logger);
		// Create root node.
		uint32_t root_id = 0;
		assembly_tree.CreateAssemblyTreeRoot(root_id);
		assembly_tree.SetNodeName(root_id, "con-rod");

		uint32_t child_node = 0;
		assembly_tree.CreateChild(root_id, child_node);
		assembly_tree.SetNodeName(child_node, "con-rod");
		SC::Store::InstanceInc mesh_instance = SC::Store::InstanceInc(model_inclusion_key, instance_key);

		uint32_t bodyInstanceNodeId;
		assembly_tree.CreateAndAddBodyInstance(child_node, bodyInstanceNodeId);
		assembly_tree.SetBodyInstanceMeshInstanceKey(bodyInstanceNodeId, mesh_instance);

		assembly_tree.SerializeToModel(model);

		// Prepare the model for streaming.
		model.PrepareStream();
		model.GenerateSCSFile((cache_path + "\\" + model_name + ".scs").c_str());
	}
	catch (std::exception const & e)
	{
		std::string message("Exception: ");
		message.append(e.what());
		message.append("\n");
		logger.Message(message.c_str());
		return 1;
	}

	return 0;
}


