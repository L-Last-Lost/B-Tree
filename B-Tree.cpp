#include <iostream>  
#include <cmath>
#include "B-Tree.h"
using namespace std;

int main() {
	BTree t(16);
	for(int i = 1; i <= 200; i++){
		t.insert(Data(i));
		// t.traverse();
	}
	
	cout << "遍历B树: \n";
	t.traverse();
	cout << endl;
	
	t.remove(6);
	cout << "删除6后的B树遍历: \n";
	t.traverse();
	cout << endl;
	
	t.remove(4);
	t.remove(8);
	t.remove(12);
	t.remove(16);
	
	cout << "最终B树遍历: \n";
	t.traverse();
	cout << endl;
	
	return 0;
}

