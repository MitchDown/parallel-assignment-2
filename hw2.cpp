/**
 * @defgroup   HW2 homework2
 *
 * @brief      This file implements homework 2.
 *
 * @author     Mitch Downey
 * @date       Jan 21, 2020
 */
#include <vector>
#include <iostream>
#include <chrono>
#include <future>
#include <functional>
using namespace std;
typedef vector<int> Data;

/**
 * @brief      This class describes a heaper.
 */
class Heaper
{
	public:
		Heaper(const Data *data) : n(data->size()), data(data)
		{
			interior = new Data(n-1,0);
		}

		virtual	~Heaper()
		{
			delete interior;
		}
	protected:
		int n; //the number of leaf nodes
		const Data *data; //vector containing leaf nodes
		Data *interior; //vector containing interior nodes

		virtual int size()
		{
			return (n-1) + n;
		}

		/**
		 * @brief      returns the value at index i
		 *
		 * @param[in]  i     index of heap element
		 *
		 * @return     value at index i
		 */
		virtual int value(int i)
		{
			if(i < n-1)
				return interior->at(i);
			else
				return data->at(i-(n-1));
		}

		/**
		 * @brief      Determines whether the specified index is leaf.
		 *
		 * @param[in]  i     index of heap element
		 *
		 * @return     True if the specified i is leaf, False otherwise.
		 */
		virtual bool isLeaf(int i)
		{
			return (i >= n-1);
		}

		/**
		 * @brief      Get the left child of parent 'i'
		 *
		 * @param[in]  i     index of parent node
		 *
		 * @return     index of i's left child
		 */
		virtual int left(int i)
		{
			return (2*i) + 1;
		}

		/**
		 * @brief      Get the right child of parent 'i'
		 *
		 * @param[in]  i     index of parent node
		 *
		 * @return     index of i's right child
		 */
		virtual int right(int i)
		{
			return (2*i) + 2;
		}

		/**
		 * @brief      Get the index of the parent of 'i'
		 *
		 * @param[in]  i     index of child node
		 *
		 * @return     index of i's parent
		 */
		virtual int parent(int i)
		{
			return i > 0 ? (i-1)/2 : 0;
		}
};

/**
 * @brief      This class describes a pairwise sum heap that can compute the prefix sum of its heap. It extends the Heaper base class.
 */
class SumHeap : public Heaper
{
	public:	
		SumHeap(const Data *data):Heaper(data)
		{
			calcSum(0);
		}

		~SumHeap()
		{

		}

		int sum(int node=0)
		{
			return value(node);
		}

		void prefixSums(Data *prefix)
		{
			parallelPrefix(prefix, 0, 0, 0);
		}
	private:
		const int MAX_DEPTH = 6;

		/**
		 * @brief      Populates the vector 'prefix' with the prefix sums of the object's contained heap
		 *
		 * @param      prefix        The prefix output vector
		 * @param[in]  root          The root index
		 * @param[in]  parentPrefix  The parent's prefix sum
		 * @param[in]  leftValue     The left sibiling's value
		 */
		void parallelPrefix(Data *prefix, int root, int parentPrefix, int leftValue)
		{
			//base case
			if(isLeaf(root))
			{
				prefix->at(root-(n-1)) = parentPrefix + leftValue + value(root);
				return;
			}
			int myPrefix = parentPrefix + leftValue;

			//if root is within the first 4 levels of the heap, spawn a thread to calculate the right child
			//otherwise, do it sequentially
			if (root <= MAX_DEPTH)
			{
				auto handle = async(&SumHeap::parallelPrefix, this, prefix, right(root), myPrefix, value(left(root)));
				parallelPrefix(prefix, left(root), myPrefix, 0);
				handle.wait();
			}
			else
			{	
				parallelPrefix(prefix, left(root), myPrefix, 0);
				parallelPrefix(prefix, right(root), myPrefix, value(left(root)));	
			}
			
		}

		/**
		 * @brief      Calculates the pairwise sum of the heap's interior nodes.
		 *
		 * @param[in]  i     index at which to calculate the pairwise sum
		 */
		void calcSum(int i)
		{
			//base case
			if(isLeaf(i))
				return;
			//if i is within the first 4 levels of the heap, spawn a thread to calculate the right child
			//otherwise, do it sequentially
			if(i <= MAX_DEPTH)
			{
				auto handle = async(&SumHeap::calcSum, this, right(i));
				calcSum(left(i));
				handle.wait();
			}
			else
			{
				calcSum(right(i));
				calcSum(left(i));
			}
			interior->at(i) = value(left(i)) + value(right(i));
		}
};

const int N = 1<<26;  // FIXME must be power of 2 for now

int main() {
    Data data(N, 1);  // put a 1 in each element of the data array
    Data prefix(N, 1);

    // start timer
    auto start = chrono::steady_clock::now();

    SumHeap heap(&data);
    heap.prefixSums(&prefix);

    // stop timer
    auto end = chrono::steady_clock::now();
    auto elpased = chrono::duration<double,milli>(end-start).count();

    int check = 1;
    for (int elem: prefix)
        if (elem != check++) {
            cout << "FAILED RESULT at " << check-1;
            break;
        }
    cout << "in " << elpased << "ms" << endl;
    return 0;
}