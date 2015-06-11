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
 
#include <iostream>
#include <queue>
#include <cmath>
#include <bitset>
#include "stack.cpp"

#define height 20
#define width 30
#define submatrices 64
#define m 8
#define e 40

using namespace std;

bool generator_matrix[width * submatrices][height * submatrices];
bool received_sequence[width * submatrices];
bool received_information_bits[height * submatrices];
bool facit_received_information_bits[submatrices * width];
bool error[submatrices * width];

int m_e, m_c, avg_nodes = 0, runs = 0;

void generate_tailbiting_matrix() {
	for (int block = 0; block < submatrices; block++) {
		for (int x = 0; x < height; x++)
		generator_matrix[block * width + x][(block + m - 1 % submatrices) * height + x] = 1;
		for (int x = height; x < width; x++) {
			for (int d_block = 0; d_block < m; d_block++) {
				for (int y = 0; y < height; y++) {
					generator_matrix[(((block) * width + x + (width * submatrices))) % (width * submatrices)][((((block + m - 1 % submatrices) - d_block) * height + y + (height * submatrices))) % (height * submatrices)] = (1 + (d_block > 0) * rand()) % 2;
				}
			}
		}
	}
}

void generate_random_received_sequence() {
	int l;
	for (int i = 0; i < height * submatrices; i++) {
		received_information_bits[i] = rand() % 2;
		facit_received_information_bits[i] = received_information_bits[i];
	}
	for (int j = 0; j < width * submatrices; j++) {
		l = 0;
		for (int i = 0; i < height * submatrices; i++)
		l ^= generator_matrix[j][i] & received_information_bits[i];
		received_sequence[j] = l;
	}
}

void add_errors() {
	int i, errors = 0;
	for (i = 0; i < submatrices * width; i++) error[i] = 0;

	while (errors < e) {
		i = rand() % (submatrices * width);
		if (error[i] == 0) {
			error[i] = 1;
			received_sequence[i] ^= 1;
			errors++;

			if ((i % width) < height) {
				received_information_bits[(i * 20) / 30 + (i % width)] ^= 1;
			}
		}
	}
}

class decoder {

	int i, j, k, l, err, nodes_generated;

	/* Primitives */
	ordered_stack L;
	stack_element new_element, old_element;

	public:

	decoder() {

		/* Calculate fano metrical constants */
		m_c = (log2(1 - 1.0 * e / (width * submatrices)) + (1.0 - 1.0 * height / width)) / (abs(log2(1 - 1.0 * e / (width * submatrices)) + (1.0 - 1.0 * height / width)));
		m_e = (log2(1.0 * e / (width * submatrices)) + (1.0 - 1.0 * height / width)) / (abs(log2(1 - 1.0 * e / (width * submatrices)) + (1.0 - 1.0 * height / width)));

		nodes_generated = 0;

		/* Copy ALL information bits */
		memcpy(new_element.information_bits, received_information_bits, submatrices * height);

		/* Set root node */
		new_element.metric = 0;
		new_element.shell = 0;
		new_element.errors = 0;
		new_element.length = m;

		/* ... and push it to the stack */
		L.push(new_element);
	}

	void update_metric(stack_element * s, stack_element * f) {

		err = 0;
		for (j = 0; j < width; j++) {
			l = 0;
			for (i = 0; i < height * submatrices; i++)
			l ^= generator_matrix[((s - > length - m) * width + j) % (width * submatrices)][i] & s - > information_bits[i];
			err += (l ^ received_sequence[((s - > length - m) * width + j) % (width * submatrices)]);
		}

		/* Fano-metrical update */
		s - > metric = f - > metric + (width - err) * m_c + err * m_e;
		s - > errors = f - > errors + err;
	}

	bool step() {

		/* Find best element */
		old_element = L.top();
		L.pop();

		/* Check if the top element satisfies the stopping conditions */
		if (old_element.length == submatrices) {
			int errors_in_decoded = 0;

			for (i = 0; i < old_element.length * height; i++)
			errors_in_decoded += (old_element.information_bits[i] ^ facit_received_information_bits[i]);

			avg_nodes = (avg_nodes + nodes_generated);
			runs++;
			cout << nodes_generated << " / avg: " << avg_nodes / runs << " out of " << runs << " runs currently with " << errors_in_decoded << " errors " << endl;

			return false;
		}

		/* Copy information bits and adjust length */
		new_element.length = old_element.length + 1;
		memcpy(new_element.information_bits, old_element.information_bits, new_element.length);

		/* Setup shell variables */
		int radius = old_element.shell, pos[radius + 2];
		for (j = 0; j < radius; j++) pos[j] = j;
		pos[radius] = height;
		pos[radius + 1] = 0;

		/* Create shell */
		do {
			/* Generate combinations */
			k = 0;
			while (pos[k] + 1 == pos[k + 1]) pos[k++] = k;
			pos[k]++;

			/* Flip shell */
			for (j = 0; j < radius; j++)
			new_element.information_bits[old_element.length * height + pos[j]] ^= 1;

			/* Find metric */
			update_metric( & new_element, & old_element);
			new_element.shell = 0;

			/* Store the element in the stack */
			if (new_element.errors <= e) L.push(new_element);

			nodes_generated++;

			/* Flip back shell */
			for (j = 0; j < radius; j++) new_element.information_bits[old_element.length * height + pos[j]] ^= 1;

		} while (k < radius);

		/* Increase old element radius and adjust its metric */
		old_element.shell++;
		old_element.metric += m_e;

		if (old_element.shell < 6) L.push(old_element);

		return true;

	}

	void flush() {
		L.flush();
	}
};

int main() {
	/* Initialize parameters */
	generate_tailbiting_matrix();

	while (1) {
		generate_random_received_sequence();
		decoder * C = new decoder();
		add_errors();
		while (C - > step());
	}

	for (int j = 0; j < height * submatrices; j++) cout << received_information_bits[j] << " ";
	cout << endl;
}