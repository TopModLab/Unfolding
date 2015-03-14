#ifndef _WXN_GRAPH_H_
#define _WXN_GRAPH_H_

#include<iostream>
#include<vector>
#include<string>
#include <queue>
#include <algorithm>
#include <list>
#include <map>
#include <fstream>
#include <assert.h>
#include <cfloat>

template<typename T>  class SparseGraph{
private:
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
    void print(){
      printf("head of svg : num_of_vertex %d\n" , num_of_vertex);
    }
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
    void print(){
      printf("source_index %d\n" , source_index);
      printf("neighbor_num %d\n" , neighbor_num);
    }
  };

  struct BodyPartOfSVG{
    int dest_index;
    float dest_dis;
    BodyPartOfSVG(){}
    BodyPartOfSVG(int _dest_index , float _dest_dis):
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
    BodyPartOfSVGWithK(int _dest_index , float _dest_dis , int _rank_k):
      BodyPartOfSVG(_dest_index , _dest_dis),
      rank_k(_rank_k){}
    bool operator<(const BodyPartOfSVGWithK& other)const{
      return rank_k < other.rank_k;
    }
  };


protected:
  std::vector<std::vector<int>> graph_neighbor;
  int node_number_;
  std::vector<std::vector<T>> graph_neighbor_dis;
public:
  SparseGraph(){
    node_number_ = -1;
  }

  void initialize(int _node_number) {
    node_number_ = _node_number;
    graph_neighbor.reserve(_node_number);
    graph_neighbor.resize(_node_number);
    graph_neighbor_dis.reserve(_node_number);
    graph_neighbor_dis.resize(_node_number);
  }

  void allocate_for_neighbor(int u , int number_of_neighbor) {
    graph_neighbor[u].reserve(number_of_neighbor);
    graph_neighbor_dis[u].reserve(number_of_neighbor);
  }
  void addedge(int u , int v , T w) {
    //u , v is the two edge
    // w is the distance
    assert(u < node_number_ && v < node_number_);
    graph_neighbor[u].push_back(v);
    graph_neighbor_dis[u].push_back(w);
  }

  int NodeNum() {
    return node_number_;
  }

  int read_svg_file_binary(const std::string& svg_file_name) {

    std::ifstream input_file (svg_file_name, std::ios::in | std::ios::binary);
    HeadOfSVG head_of_svg;
    input_file.read( (char*)&head_of_svg , sizeof(head_of_svg));
    head_of_svg.print();
    initialize(head_of_svg.num_of_vertex);

    double average_neighbor_number(0.0);
    double max_radius_in_file(0.0);
    double average_radius(0.0);

    for(int i = 0; i < head_of_svg.num_of_vertex;++i){
      BodyHeadOfSVG body_head;
      input_file.read( (char*)&body_head , sizeof(body_head));
      average_neighbor_number += (double)body_head.neighbor_num;
      std::vector<BodyPartOfSVG> body_parts;
      for(int j = 0; j < body_head.neighbor_num;++j){
        BodyPartOfSVG body_part;
        input_file.read((char*)&body_part , sizeof(body_part));
        body_parts.push_back(body_part);
      }
      allocate_for_neighbor(body_head.source_index , body_parts.size());
      for (int j = 0; j < body_parts.size();++j) {
        addedge(body_head.source_index ,
          body_parts[j].dest_index ,
          body_parts[j].dest_dis);
      }
      if( i > 0 && i % (head_of_svg.num_of_vertex / 10 ) == 0 ){
        std::cerr << "read " << i * 100 / head_of_svg.num_of_vertex  << " percent \n";
      }
    }

    std::cerr << "reading done..\n";
    input_file.close();


    return 0;
  }


  virtual void findShortestDistance(int source)=0;
  virtual int getSource(int v)=0;
  virtual T distanceToSource(int index)=0;
};


template <typename T> class LC_LLL:public SparseGraph<T>{

private:

  std::vector<T> dis;
  std::vector<bool> visited;
  std::vector<int> fathers;

public:
  LC_LLL(){}

  void getPath(const int& dest , std::vector<int>& path_nodes) {
    path_nodes.clear();
    int u = dest;
    while( u != fathers[u] ){
      path_nodes.push_back(u);
      u = fathers[u];
    }
    path_nodes.push_back(u);
    std::reverse(path_nodes.begin() , path_nodes.end());
  }

  
  void findShortestDistance(int source)
  {
    fathers.resize(this->node_number_);
    fill(fathers.begin(),fathers.end(),-1);
    dis.resize(this->node_number_);
    fill(dis.begin(), dis.end(), FLT_MAX);
    visited.resize(this->node_number_);
    fill(visited.begin(), visited.end(), false);
    std::deque<int> que;
    double dis_sum = 0.0;
    dis[source] = 0;
    que.push_back(source);
    fathers[source] = source;
    visited[source] = true;

    while (!que.empty()) {
      int u = -1;
      while (true) {

        if (dis[que.front()] > dis_sum / que.size()) {
          que.push_back(que.front());
          que.pop_front();
        } else {
          u = que.front();
          que.pop_front();
          dis_sum -= dis[u];
          visited[u] = false;
          break;
        }
      }
      for (int i = 0; i < this->graph_neighbor[u].size(); ++i) {
        if (dis[this->graph_neighbor[u][i]] > dis[u] + this->graph_neighbor_dis[u][i]) {
          int v = this->graph_neighbor[u][i];
          T w = this->graph_neighbor_dis[u][i];
          double old_v = dis[v];
          dis[v] = dis[u] + w;
          fathers[v] = u;
          if (visited[v] == false) {
            que.push_back(v);
            visited[v] = true;
            dis_sum += dis[v];
          } else {
            dis_sum -= old_v;
            dis_sum += dis[v];
          }
        }
      }
    }
  }

  inline int getSource(int v){
    if( fathers[v] == v ){
      return v;
    }else{
      fathers[v] = getSource(fathers[v]);
      return fathers[v];
    }
  }

  inline T distanceToSource(int index) {
    if(index < 0 || index >= this->node_number_ ){
      std::cerr << "wrong index " << index << "\n";
      return 0;
    }
    return dis[index];
  }

};

#endif
