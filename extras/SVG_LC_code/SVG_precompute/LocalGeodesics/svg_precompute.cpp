#include "stdafx.h"
#include "wxnTime.h"
#include "ICHWithFurtherPriorityQueue.h"
#include "svg_precompute.h"

struct HeadOfSVG {
	int begin_vertex_index;
	int end_vertex_index;
	int num_of_vertex;
	HeadOfSVG(int _begin_vertex_index , 
		int _end_vertex_index , 
		int _num_of_vertex)
		:
	begin_vertex_index(_begin_vertex_index) ,
		end_vertex_index(_end_vertex_index) , 
		num_of_vertex(_num_of_vertex){}
	HeadOfSVG(){}
};

struct BodyHeadOfSVG{
	int source_index;
	int neighbor_num;
	BodyHeadOfSVG(int _source_index , int _neighbor_num)
	{
		source_index = _source_index;
		neighbor_num = _neighbor_num;
	}
	BodyHeadOfSVG(){}
};
struct BodyPartOfSVG{
	int dest_index;
	float dest_dis;
	BodyPartOfSVG(){}
	BodyPartOfSVG(int _dest_index , float _dest_dis ):
		dest_index(_dest_index),
		dest_dis(_dest_dis)
	{
		dest_index = _dest_index;
		dest_dis = _dest_dis;
	}
};
struct BodyPartOfSVGWithK : BodyPartOfSVG{
	int rank_k;
	BodyPartOfSVGWithK(){}
	BodyPartOfSVGWithK(int _dest_index , float _dest_dis , int _rank_k ):
		BodyPartOfSVG(_dest_index , _dest_dis),
		rank_k(_rank_k){}
	bool operator<(const BodyPartOfSVGWithK& other)const{
		return rank_k < other.rank_k;
	}
};



bool flag_first_output = true;
void generate_output(const string& output_file_name,stringstream& output_str,bool force_output = false)
{
	if( force_output || output_str.tellp() > 50 * 1024 * 1024 ){
		FILE* fout = NULL;
		if(flag_first_output){
			fout = fopen(output_file_name.c_str(),"w");
			flag_first_output = false;
		}else{
			fout = fopen(output_file_name.c_str(),"a");
		}
		const string& str = output_str.str();
		ElapasedTime time;
		fwrite(str.c_str(),1,str.length(),fout);
		fclose(fout);
		cerr << "write to file time " << time.getTime() << "\n";
		output_str.str(std::string());
		output_str.clear();
	}
}



void svg_precompute(const string& input_obj_name, const int fixed_k, string& svg_file_name) {
	//Step 1. Initialize models
	CRichModel model(input_obj_name);
	model.Preprocess();

	int begin_vertex_index = 0;

	int end_vertex_index = model.GetNumOfVerts() - 1;

	//string output_file_name;
	if (begin_vertex_index == 0 && end_vertex_index == model.GetNumOfVerts() - 1) {
		svg_file_name = input_obj_name.substr(0,input_obj_name.length() - 4 ) 
			+ "_SVG_k" + to_string(fixed_k) +  ".binary";
	}

	// see if the file exists, if so, skip the computation
	if (std::ifstream(svg_file_name).good()) {
		cout << "SVG file exists, skipping SVG computation." << endl;
		return;
	}

	ofstream output_file (svg_file_name.c_str() , ios::out | ios::binary);
	int num_of_vertex = end_vertex_index - begin_vertex_index + 1;
	HeadOfSVG head_of_svg(begin_vertex_index , end_vertex_index , num_of_vertex );     
	output_file.write((char*)&head_of_svg , sizeof(head_of_svg));

	ElapasedTime time_once;
	vector<int> idx(model.GetNumOfVerts());
	for(int i = 0; i < model.GetNumOfVerts(); ++i){
		idx[i] = i;
	}
	srand(201305);

	double dis_time_total(0);
	double past_time(0);
	//#pragma omp parallel for
	for (int tmp_source = begin_vertex_index;tmp_source <= end_vertex_index;++tmp_source) {

		if (time_once.getTime() -  past_time > 5 ) {
			past_time = time_once.getTime();
			char buf[128];
			sprintf(buf, "Computed %.0lf percent", (double) tmp_source  * 100. / (end_vertex_index - begin_vertex_index));
			time_once.printTime(buf );
		}
		ElapasedTime dis_time;
		int source_index = idx[tmp_source];
		double farthest = 0.0;
		//Step 2: Construct a group of source points;
		vector<int> sources;
		sources.push_back(source_index);
		//Step 3: Construct a new algorithm object
		CICHWithFurtherPriorityQueue alg(model, sources);
		//Step 4: Locally propagate wavefronts and stop at the prescribed geodesic distance.
		set<int> fixedDests;
		//The first parameter is the distance threshold, 
		//and the second is to return those vertices where the geodesic distance makes sense.
		double max_radius = 1e10;
		alg.ExecuteLocally_SVG(max_radius, fixedDests,fixed_k);
		dis_time_total += dis_time.getTime();
		//printf("Totally collected: %d\n", fixedDests.size());
		set<int>::iterator itr;
		int cnt = 0;
		vector<pair<int,double>> dests;

		struct node {
			int id;
			double dis;
			int operator<(const node & other) const{
				return dis < other.dis;
			}
		};
		vector<node> covered_points;
		covered_points.resize(fixedDests.size());
		int _index = 0;
		for (itr = fixedDests.begin(); itr != fixedDests.end(); ++itr) {
			int v = *itr;
			map<int, CICHWithFurtherPriorityQueue::InfoAtVertex>::const_iterator it = alg.m_InfoAtVertices.find(v);
			int indexOfParent = it->second.indexOfParent;
			int parent = 0;
			covered_points[_index].id = v;
			covered_points[_index].dis = it->second.disUptodate;
			_index ++;
			if (farthest < it->second.disUptodate){
				farthest = it->second.disUptodate;
			}
			if (it->second.fParentIsPseudoSource) {
				parent = indexOfParent;
			} else {
				parent = it->second.indexOfRootVertOfParent;
			}
			if (parent == sources[0]) {
				dests.push_back(pair<int,double>(v , alg.m_InfoAtVertices[v].disUptodate));
				++cnt;
			}
		}
		std::sort(covered_points.begin(), covered_points.end());
		std::map<int, int> mp;
		for(int i = 0; i < covered_points.size(); ++i){
			mp[covered_points[i].id] = i;
		}

		BodyHeadOfSVG body_header(source_index , dests.size());
		output_file.write((char*)&body_header , sizeof(body_header));

		vector<BodyPartOfSVGWithK> body_parts(dests.size());
		for(int i = 0; i < dests.size(); ++i) {
			BodyPartOfSVGWithK body_part(dests[i].first , dests[i].second , mp[dests[i].first]);
			body_parts[i] = body_part;
		}
		sort(body_parts.begin() , body_parts.end());
		for (int i = 0; i < body_parts.size(); ++i) {
			BodyPartOfSVG b = body_parts[i];
			output_file.write( (char*)&b , sizeof(b));
		}
	}

	time_once.printTime("time past ");
	output_file.close();

}
