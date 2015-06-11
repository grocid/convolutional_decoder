/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2012-2015 Carl Löndahl
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

using namespace std;

struct stack_element {

	int length, metric, errors, shell, number, from_number, last_flipped, last_flipped2, rad;
	bool information_bits[height * submatrices];
	bool correct_one;

	bool operator < (const stack_element & rhs) const {
		return metric < rhs.metric;
	}

	stack_element() {
		shell = 0;
	}
};


class ordered_stack {

	int size, max_size;
	priority_queue <stack_element> pq_stack;

	public:

	ordered_stack() {
		size = 0;
	}

	stack_element top() {
		return pq_stack.top();
	}

	void push(stack_element & s) {
		size++;
		pq_stack.push(s);
	}

	void ssize() {
		cout << "Size is " << pq_stack.size() << endl;
	}

	void pop() {
		size--;
		pq_stack.pop();
	}

	bool empty() {
		return (size == 0);
	}

	void flush() {
		int i, j = 0;
		while (size > 0) {
			if (pq_stack.top().correct_one) {
				cout << j << " " << pq_stack.top().metric << " @" << pq_stack.top().length << " *\t";
				for (i = 0; i < pq_stack.top().length * height; i++)
				cout << (pq_stack.top().information_bits[i] ^ facit_received_information_bits[i]);
				cout << "\t " << pq_stack.top().shell << " " << pq_stack.top().number << " from " << pq_stack.top().from_number << " lf " << pq_stack.top().last_flipped << "&" << pq_stack.top().last_flipped2 << " rad " << pq_stack.top().rad << endl;
			}
			pq_stack.pop();
			size--;
			++j;
		}
	}
};