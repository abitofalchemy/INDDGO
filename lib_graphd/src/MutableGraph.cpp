/*
  This file is part of INDDGO.

  Copyright (C) 2012, Oak Ridge National Laboratory 

  This product includes software produced by UT-Battelle, LLC under Contract No. 
  DE-AC05-00OR22725 with the Department of Energy. 

  This program is free software; you can redistribute it and/or modify
  it under the terms of the New BSD 3-clause software license (LICENSE). 
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  LICENSE for more details.

  For more information please contact the INDDGO developers at: 
  inddgo-info@googlegroups.com

*/



#include "MutableGraph.h"
#include "Log.h"
#include "Debug.h"
#include "GraphProperties.h"
#include <string.h>

namespace Graph
{
	MutableGraph::MutableGraph()
	{
		simple = true;
		canonical = true;
		key = 0;
	}

	MutableGraph::~MutableGraph()
	{
	}

	bool MutableGraph::is_canonical() const
	{
		return canonical;
	}

	bool MutableGraph::is_simple() const
	{
		return simple;
	}

	void MutableGraph::set_canonical(bool c)
	{
		canonical = c;
	}

	void MutableGraph::add_edge_advance(int u, int v)
	{
		if (v < 0 || u < 0 || v >= capacity || u >= capacity)
			fatal_error(
			"%s:  remove_vertex() called with vertices %d and %d but there are %d connected nodes\n",
			__FUNCTION__, u, v, capacity);

		if (nodes[u].label == -1)
			fatal_error("%s:  Tried to use  GD_UNDEFINED vertex %d\n",
			__FUNCTION__, u);

		if (nodes[v].label == -1)
			fatal_error("%s:  Tried to use GD_UNDEFINED vertex %d\n", __FUNCTION__,
			v);

		if (simple == true)
		{
			list<int>::iterator it;
			if (u == v)
			{
				simple = false;
				canonical = false;
			}
			else
			{
				it = nodes[u].nbrs.begin();
				while (it != nodes[u].nbrs.end())
				{
					if (*it == v)
					{
						simple = false;
						canonical = false;
						break;
					}
					++it;
				}

				if (simple == true)
				{
					it = nodes[v].nbrs.begin();
					while (it != nodes[v].nbrs.end())
					{
						if (*it == u)
						{
							simple = true;
							canonical = false;
							break;
						}
						++it;
					}

				}

			}

		}

		nodes[u].nbrs.push_back(v);
		if (u != v)
			nodes[v].nbrs.push_back(u);

		degree[u]++;
		if (u != v)
			degree[v]++;

		num_edges++;
		if (num_connected_components > 1)
			num_connected_components = -1;

	}
	bool MutableGraph::remove_edge(int u, int v)
	{
		if (v < 0 || u < 0 || v >= capacity || u >= capacity)
		{
			fatal_error(
				"%s:  remove_vertex() called with vertices %d and %d but there are %d connected nodes\n",
				__FUNCTION__, u, v, capacity);
		}
		if (nodes[u].label == -1)
			fatal_error("%s:  Tried to remove GD_UNDEFINED vertex %d\n",
			__FUNCTION__, u);

		if (nodes[v].label == -1)
			fatal_error("%s:  Tried to remove GD_UNDEFINED vertex %d\n",
			__FUNCTION__, v);

		list<int>::iterator it;
		bool found = false;
		it = nodes[u].nbrs.begin();
		while (it != nodes[u].nbrs.end())
		{
			if (*it == v)
			{
				nodes[u].nbrs.erase(it);
				found = true;
				break;
			}
			++it;
		}

		it = nodes[v].nbrs.begin();
		while (it != nodes[v].nbrs.end())
		{
			if (*it == u)
			{
				nodes[v].nbrs.erase(it);
				found = true;
				break;
			}
			++it;
		}

		num_connected_components = -1;
		if (found)
		{
			degree[u]--;
			degree[v]--;
			num_edges--;
			return true;
		}
		return false;
	}

	void MutableGraph::remove_vertex(int v)
	{
		if (!canonical)
			print_message(0, "%s:  Graph must be in canonical form\n", __FUNCTION__);

		if (v < 0 || v >= capacity)
			fatal_error(
			"%s:  remove_vertex() called with vertex %d and there are %d connected nodes\n",
			__FUNCTION__, v, capacity);

		if (nodes[v].label == -1)
			fatal_error("%s:  Tried to remove GD_UNDEFINED vertex %d\n",
			__FUNCTION__, v);

		if (nodes[v].nbrs.size() != 0)
		{
			list<int>::iterator i, j;
			int u, len;
			num_edges = num_edges - nodes[v].nbrs.size();
			for (i = nodes[v].nbrs.begin(); i != nodes[v].nbrs.end(); ++i)
			{
				u = *i;
				len = nodes[u].nbrs.size();
				nodes[u].nbrs.remove(v);
				if ((int) ((((nodes[u].nbrs.size())))) == len)
					fatal_error(
					"%s: Didn't find node %d (mask=%d) in adjacency list of %d\n",
					__FUNCTION__, v, u);

				degree[u] = degree[u] - (len - (int) ((((nodes[u].nbrs.size())))));
			}
			nodes[v].nbrs.clear();
		}

		degree[v] = 0;
		nodes[v].label = -1;
		num_nodes--;
	}


	// doesn't currently check that uv is an edge
	int MutableGraph::edge_subdivision(int u, int v, int w)
	{
		int temp=0;
		Node *new_graph_node;
		if (nodes[u].label == -1 || nodes[v].label == -1)
	    {
			fatal_error(
				"%s: Cannot remove edge (%d, %d) as one of its vertices is undefined!\n",
				__FUNCTION__, u, v);
	    }

		// if w provided, check that it is in bounds
		if(w > capacity)
		{
			nodes.resize(2*capacity);
			capacity*=2;
//                 FERROR("%s: element is out of bounds", __FUNCTION__);
//                 throw GraphException("element is out of bounds\n");
		}

		// add a node if necessary
		if(w==-1)
		{
			return w;
                //temp=nodes.size()+1;
                //Node newnode
                //
                // not yet implemented
		}

		else
		{
			nodes[w].nbrs.clear();
			if(!nodes[w].nbrs.empty())
			{
				FERROR("%s: node is not empty", __FUNCTION__);
				throw GraphException("node is not empty\n");
			}
			else
			{
				nodes[w].label=w;
				degree[w] = 2;
				nodes[w].nbrs.push_back(u);
				nodes[w].nbrs.push_back(v);

				list<int>::iterator it;
				// remove u from v's nbrs list and vice versa
				for(it = nodes[u].nbrs.begin() ; it != nodes[u].nbrs.end();it++)
				{
					if(*it=v) {*it=w; break;}
				}
				for(it = nodes[v].nbrs.begin() ; it != nodes[v].nbrs.end();it++)
				{
					if(*it=u) {*it=w;break;}
				}
			}
		}
		num_edges++;
		next_label++;
		num_nodes++;
		return w;
	}


	int MutableGraph::contract_edge(int u, int v)
	{
		int i;
		if (simple != true)
		{
			fatal_error("%s: called on a non-simple graph!\n", __FUNCTION__);
		}
		if (nodes[u].label == -1 || nodes[v].label == -1)
		{
			fatal_error(
				"%s: Cannot remove edge (%d, %d) as one of its vertices is undefined!\n",
				__FUNCTION__, u, v);
		}
		vector<bool> neighbors(capacity);
		fill(neighbors.begin(), neighbors.end(), false);
		list<int>::iterator it;
		bool foundv = false;
		it = nodes[u].nbrs.begin();
		while (it != nodes[u].nbrs.end())
		{
			neighbors[*it] = true;
			if (*it == v)
				foundv = true;

			++it;
		}
		if (foundv == false)
			return false;

		it = nodes[v].nbrs.begin();
		while (it != nodes[v].nbrs.end())
		{
			neighbors[*it] = true;
			++it;
		}
		remove_vertex(u);
		remove_vertex(v);
		nodes[u].label = next_label;
		next_label++;
		num_nodes++;
		for (i = 0; i < capacity; i++)
		{
			if (i != u && i != v)
			{
				if (neighbors[i] == true)
				{
					nodes[i].nbrs.push_back(u);
					nodes[u].nbrs.push_back(i);
					degree[u]++;
					degree[i]++;
					num_edges++;
				}
			}

		}

		return u;
	}

//////DAG: fuse_vertices(int, int)
////// similar to contract_edge, but u,v don't need to have an edge between them
	int MutableGraph::fuse_vertices(int u, int v)
	{
		int i;
		if (simple != true)
		{
			fatal_error("%s: called on a non-simple graph!\n", __FUNCTION__);
		}
		if (nodes[u].label == -1 || nodes[v].label == -1)
		{
			fatal_error(
				"%s: Cannot remove edge (%d, %d) as one of its vertices is undefined!\n",
				__FUNCTION__, u, v);
		}
		vector<bool> neighbors(capacity); //what is capacity?
		fill(neighbors.begin(), neighbors.end(), false);
		list<int>::iterator it;

		if (v == u)
			return false;

		it = nodes[u].nbrs.begin();
		while (it != nodes[u].nbrs.end())
		{
			neighbors[*it] = true;
			++it;
		}
		it = nodes[v].nbrs.begin();
		while (it != nodes[v].nbrs.end())
		{
			neighbors[*it] = true;
			++it;
		}
		int templabel = nodes[u].label;
		list<int> singlepoint;
		list<int> anothersinglepoint;

		remove_vertex(u);
		remove_vertex(v);
		nodes[u].label = next_label;
		next_label++;
		num_nodes++;

		for (i = 0; i < capacity; i++)
		{
			if (i != u && i != v)
			{
				if (neighbors[i] == true)
				{
					nodes[i].nbrs.push_back(u);
					nodes[u].nbrs.push_back(i);
					degree[u]++;
					degree[i]++;
					num_edges++;
				}
			}

		}

		return u;
	}

	void MutableGraph::resize_graph(int size)
	{
		if(size < capacity)
		//if(size < 1)
		{
			fatal_error(
			//"%s:  resize_graph() called with new size %d less than 1\n",
			//__FUNCTION__, size);
			"%s:  resize_graph() called with new size %d less than capacity %d\n",
			__FUNCTION__, size, capacity);
		}
		else
		{
			nodes.resize(size,Node());
			degree.resize(size,0);
	        this->set_capacity(size);
		}
	}

	void MutableGraph::set_simple(bool si)
	{
		simple = si;
	}

	void MutableGraph::add_edge(int u, int v)
	{
		if (v < 0 || u < 0 || v >= capacity || u >= capacity)
			fatal_error(
			"%s:  remove_vertex() called with vertices %d and %d but there are %d connected nodes\n",
			__FUNCTION__, u, v, capacity);

		if (nodes[u].label == -1)
			fatal_error("%s:  Tried to use  GD_UNDEFINED vertex %d\n",
			__FUNCTION__, u);

		if (nodes[v].label == -1)
			fatal_error("%s:  Tried to use GD_UNDEFINED vertex %d\n", __FUNCTION__,
			v);

		nodes[u].nbrs.push_back(v);
		if (u != v)
			nodes[v].nbrs.push_back(u);

		degree[u]++;
		if (u != v)
			degree[v]++;

		num_edges++;
	}

	void MutableGraph::resize_adj_vec(int n)
	{
		adj_vec.resize(n, 0);
	}

	vector<int> MutableGraph::get_adj_vec() const
	{
		return adj_vec;
	}

	vector<int> *MutableGraph::get_adj_vec_ptr() 
	{
		return (&(this->adj_vec));
	}

	vector<int> MutableGraph::get_adjncy() const
	{
		return adjncy;
	}

	vector<int> MutableGraph::get_xadj() const
	{
		return xadj;
	}

	void MutableGraph::set_adjncy(vector<int> adjncy)
	{
		this->adjncy = adjncy;
	}

	MutableGraph::MutableGraph(int n) :
	Graph(n)
	{
	}

	void MutableGraph::set_xadj(vector<int> xadj)
	{
		this->xadj = xadj;
	}

	void MutableGraph::eliminate_vertex(int v, list<int> *forward_neighbors,
		bool remove)
	{
		if (!this->canonical)
			print_message(0, "%s:  Graph must be in canonical form\n", __FUNCTION__);

		if (v < 0 || v >= this->capacity)
			fatal_error(
			"%s:  eliminate_vertex() called with vertex %d and there are %d possible nodes\n",
			__FUNCTION__, v, this->capacity);

		if (this->degree[v] == 0)
			return;

		int i, j;
		GraphProperties properties;
		if (this->degree[v] == 1)
		{
			print_message(1, "Degree[%d]=1\n", v);
			j = this->nodes[v].nbrs.front();
			this->nodes[j].nbrs.remove(v);
			if (remove)
				this->nodes[v].nbrs.pop_front();

			this->num_edges--;
			this->degree[v]--;
			this->degree[j]--;
			return;
		}
		int *v_neighbors;
		char *forward_neighbor_vec;
		list<int>::iterator k;
		list<int>::iterator w;
		if (forward_neighbors != NULL)
		{
			forward_neighbor_vec = new char[this->capacity];
			memset(forward_neighbor_vec, 0, (this->capacity) * sizeof(char));

			// Create the incidence vector
			for (w = forward_neighbors->begin(); w != forward_neighbors->end(); ++w)
				forward_neighbor_vec[*w] = 1;
		}
		else
			forward_neighbor_vec = NULL;
		v_neighbors = new int[this->degree[v]];
		memset(v_neighbors, 0, this->degree[v] * sizeof(int));
		j = 0;
		for (k = this->nodes[v].nbrs.begin(); k != this->nodes[v].nbrs.end(); ++k)
		{
			v_neighbors[j] = *k;
			j++;
		}
		print_message(100, "Found %d neighbors of %d\nDeg is %d\n", j, v,
			this->degree[v]);
		for (i = 0; i < j; i++)
			print_message(100, "v_neighbors[%d]=%d\n", i, v_neighbors[i]);

		if (forward_neighbor_vec)
		{
			for (i = 0; i < this->degree[v]; i++)
			{
				if (forward_neighbor_vec[v_neighbors[i]] == 1)
				{
					int current_key = properties.fill_adj_vec(this, v_neighbors[i]);
					for (j = i + 1; j < this->degree[v]; j++)
					{
						if (forward_neighbor_vec[v_neighbors[j]] == 1)
						{
							if (this->adj_vec[v_neighbors[j]] != current_key)
							{
								this->nodes[v_neighbors[i]].nbrs.push_back(
									v_neighbors[j]);
								this->degree[v_neighbors[i]]++;
								this->nodes[v_neighbors[j]].nbrs.push_back(
									v_neighbors[i]);
								this->degree[v_neighbors[j]]++;
								this->num_edges++;
							}
						}

					}

				}

				if (remove)
				{
					this->nodes[v_neighbors[i]].nbrs.remove(v);
					this->degree[v_neighbors[i]]--;
					this->num_edges--;
				}
			}

		}
		else
		{
			for (i = 0; i < this->degree[v]; i++)
			{
				int current_key = properties.fill_adj_vec(this, v_neighbors[i]);
				for (j = i + 1; j < this->degree[v]; j++)
				{
					if (this->adj_vec[v_neighbors[j]] != current_key)
					{
						this->nodes[v_neighbors[i]].nbrs.push_back(v_neighbors[j]);
						this->degree[v_neighbors[i]]++;
						this->nodes[v_neighbors[j]].nbrs.push_back(v_neighbors[i]);
						this->degree[v_neighbors[j]]++;
						this->num_edges++;
					}
				}

				if (remove)
				{
					this->nodes[v_neighbors[i]].nbrs.remove(v);
					this->degree[v_neighbors[i]]--;
					this->num_edges--;
				}
			}

		}

		if (remove)
		{
			this->nodes[v].nbrs.clear();
			this->degree[v] = 0;
		}
		delete[] v_neighbors;
		if (forward_neighbor_vec)
			delete[] forward_neighbor_vec;

		return;
	}
}
