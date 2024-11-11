#include <iostream>  
#include <cmath>
using namespace std;

// 关键字数据
typedef struct Data{
	int key;
	// ...
	Data() : key(){}
	Data(int key) : key(key){}
	
	operator < (const Data & d1)const{
		return key < d1.key;
	}
	operator > (const Data & d1)const{
		return key > d1.key;
	}
	operator == (const Data & d1)const{
		return key == d1.key;
	}
	friend ostream& operator<<(ostream& os, const Data& dt);
}Data;

ostream& operator<<(ostream& os, const Data& dt)
{
	os << dt.key;
	return os;
}

class BTreeNode {
	Data *keys;           // 节点中的键值
	int m;               // 阶数
	BTreeNode **children; // 孩子节点数组
	int n;               // 当前存储的键数量
	bool leaf;           // 是否为叶子节点
	
public:
	BTreeNode(int _m, bool _leaf); // 构造函数
	
	void traverse(int level = 0); // 遍历节点
	
	BTreeNode *search(Data k); // 搜索键值k
	
	void insertNonFull(Data k); // 当节点不满时插入
	
	void splitChild(int i, BTreeNode *y); // 分割子节点
	
	void remove(Data k); // 删除键值k
	
	int findKey(Data k); // 查找键值的位置
	
	void removeFromLeaf(int idx); // 从叶子节点删除
	
	void removeFromNonLeaf(int idx); // 从非叶子节点删除
	
	Data getPred(int idx); // 获取前驱
	
	Data getSucc(int idx); // 获取后继
	
	void fill(int idx); // 填充孩子节点
	
	void borrowFromPrev(int idx); // 从前面的兄弟节点借
	
	void borrowFromNext(int idx); // 从后面的兄弟节点借
	
	void merge(int idx); // 合并
	
	friend class BTree;
};

class BTree {
	BTreeNode *root; // 根节点
	int m;           // 阶数
	
public:
	BTree(int _m) {
		root = nullptr;
		m = _m;
	}
	
	void traverse() {
		if (root != nullptr)
			root->traverse();
	}
	
	BTreeNode *search(Data k) {
		return (root == nullptr) ? nullptr : root->search(k);
	}
	
	void insert(Data k); // 插入操作
	
	void remove(Data k); // 删除操作
};

// 构造
BTreeNode::BTreeNode(int _m, bool _leaf) {
	m = _m;
	leaf = _leaf;
	keys = new Data[m - 1];
	children = new BTreeNode *[m];
	n = 0;
}

void BTreeNode::traverse(int level) {
	int i;
	// 打印当前节点的层级信息
	cout << string(level * 4, ' ') << "Level " << level << ": ";
	
	// 打印节点的关键字
	for (i = 0; i < n; i++) {
		cout << keys[i] << " ";
	}
	cout << endl;
	
	// 递归遍历子节点
	for (i = 0; i <= n; i++) {
		if (!leaf) {
			children[i]->traverse(level + 1);
		}
	}
}

// 递归搜索键值k，找到则返回节点指针，否则返回nullptr
BTreeNode *BTreeNode::search(Data k) {
	int i = 0;
	// 寻找第一个大于或等于k的键位置
	while (i < n && k > keys[i])
		i++;
	
	// 如果键在节点中找到，返回当前节点
	if (keys[i] == k)
		return this;
	
	// 如果节点为叶节点且没找到，返回nullptr
	if (leaf)
		return nullptr;
	
	// 否则递归搜索对应子节点
	return children[i]->search(k);
}

// 在B树中插入键值k
void BTree::insert(Data k) {
	// 如果树为空，创建新节点作为根节点
	if (root == nullptr) {
		root = new BTreeNode(m, true); // 新建叶节点
		root->keys[0] = k;             // 插入k为第一个键
		root->n = 1;                   // 更新节点键数量
	} else {
		// 如果根节点满了，树高度增加
		if (root->n == m - 1) {
			// 创建新根节点
			BTreeNode *s = new BTreeNode(m, false);
			
			// 新根的第一个子节点指向旧根
			s->children[0] = root;
			
			// 分裂旧根并调整新根
			s->splitChild(0, root);
			
			// 根据k值选择合适的子节点
			int i = 0;
			if (s->keys[0] < k)
				i++;
			s->children[i]->insertNonFull(k);
			
			// 更新根节点为新节点
			root = s;
		} else {
			// 如果根节点未满，直接插入到根节点中
			root->insertNonFull(k);
		}
	}
}

// 在非满节点中插入键值k
void BTreeNode::insertNonFull(Data k) {
	int i = n - 1;
	
	// 如果是叶节点，找到合适位置插入k
	if (leaf) {
		while (i >= 0 && keys[i] > k) {
			keys[i + 1] = keys[i];
			i--;
		}
		
		// 插入k到找到的位置
		keys[i + 1] = k;
		n = n + 1; // 更新键数量
	} else {
		// 如果不是叶节点，找到插入k的子节点
		while (i >= 0 && keys[i] > k)
			i--;
		
		// 如果子节点已满，先分裂
		if (children[i + 1]->n == m - 1) {
			splitChild(i + 1, children[i + 1]);
			
			// 再次确定子节点插入位置
			if (keys[i + 1] < k)
				i++;
		}
		children[i + 1]->insertNonFull(k); // 递归插入子节点
	}
}

// 分裂第i个满子节点y
void BTreeNode::splitChild(int i, BTreeNode *y) {
	int t = ceil(m / 2.0) - 1; // 计算中间位置
	BTreeNode *z = new BTreeNode(m, y->leaf); // 新建节点z
	z->n = t; // 设置z的键数量
	
	// 将y的后半部分键值复制到z, t + 1 就是 从中间位置的下一结点开始的下标
	for (int j = 0; j < t; j++)
		z->keys[j] = y->keys[j + t + 1];
	
	// 如果y不是叶节点，复制子节点指针
	if (!y->leaf) {
		for (int j = 0; j <= t; j++)
			z->children[j] = y->children[j + t + 1];
	}
	
	y->n = t; // 更新y的键数量
	
	// 将z插入到当前节点的子节点数组中
	for (int j = n; j >= i + 1; j--)
		children[j + 1] = children[j];
	children[i + 1] = z;
	
	// 将y的中间键移动到当前节点
	for (int j = n - 1; j >= i; j--)
		keys[j + 1] = keys[j];
	keys[i] = y->keys[t];
	n = n + 1; // 更新当前节点键数量
}

// 从B树中删除键值k
void BTree::remove(Data k) {
	// 检查B树是否为空
	if (!root) {
		cout << "The tree is empty\n";
		return;
	}
	root->remove(k); // 调用删除函数从根节点开始
	
	// 如果根节点键数量变为0，更新根节点指针
	if (root->n == 0) {
		BTreeNode *tmp = root;
		if (root->leaf)
			root = nullptr; // 根节点为空
		else
			root = root->children[0]; // 新根节点为第一个子节点
		delete tmp; // 释放旧根节点内存
	}
}

// 结点删除关键字
void BTreeNode::remove(Data k) {
	int idx = findKey(k);
	if (idx < n && keys[idx] == k) {
		if (leaf)
			removeFromLeaf(idx);
		else
			removeFromNonLeaf(idx);
	} else {
		// 搜索到叶子结点还没找到
		if (leaf) {
			cout << "The key " << k << " is not in the tree\n";
			return;
		}
		
		bool flag = ((idx == n) ? true : false);
		if (children[idx]->n < ceil(m / 2.0) - 1)
			fill(idx);
		if (flag && idx > n)
			children[idx - 1]->remove(k);
		else
			children[idx]->remove(k);
	}
}


// 在节点中找到并返回键k的位置
int BTreeNode::findKey(Data k) {
	int idx = 0;
	while (idx < n && keys[idx] < k)
		++idx;
	return idx;
}

// 从叶节点中删除索引为idx的键
void BTreeNode::removeFromLeaf(int idx) {
	for (int i = idx + 1; i < n; ++i)
		keys[i - 1] = keys[i];
	n--; // 减少键数量
}

// 从非叶节点中删除索引为idx的键
// 因为要保证非根结点的每个结点元素个数最少为 m / 2 上取整 - 1， 所以我们用元素多的结点来替换被删除元素
void BTreeNode::removeFromNonLeaf(int idx) {
	Data k = keys[idx]; // 保存要删除的键
	
	if (children[idx]->n >= ceil(m / 2.0)) {
		Data pred = getPred(idx); // 获取前驱
		keys[idx] = pred; // 用前驱替换
		children[idx]->remove(pred);
	} else if (children[idx + 1]->n >= ceil(m / 2.0)) {
		Data succ = getSucc(idx); // 获取后继
		keys[idx] = succ; // 用后继替换
		children[idx + 1]->remove(succ);
	} else {
		merge(idx); // 合并子节点
		children[idx]->remove(k);
	}
}

// 获取当前节点中键索引为 idx 的前驱键值
// 前驱键值是在当前节点左子树的最右侧节点中找到的最大键值
Data BTreeNode::getPred(int idx) {
	BTreeNode *cur = children[idx];
	while (!cur->leaf)
		cur = cur->children[cur->n];
	return cur->keys[cur->n - 1];
}

// 获取当前节点中键索引为 idx 的后继键值
// 后继键值是在当前节点右子树的最左侧节点中找到的最小键值
Data BTreeNode::getSucc(int idx) {
	BTreeNode *cur = children[idx + 1];
	while (!cur->leaf)
		cur = cur->children[0];
	return cur->keys[0];
}

// 为索引为 idx 的子节点提供额外的键值以满足 B 树的最低键数要求
// 如果左或右兄弟节点有足够的键，则从中借一个键；否则，将子节点与相邻兄弟节点合并
void BTreeNode::fill(int idx) {
	if (idx != 0 && children[idx - 1]->n >= ceil(m / 2.0))
		borrowFromPrev(idx);
	else if (idx != n && children[idx + 1]->n >= ceil(m / 2.0))
		borrowFromNext(idx);
	else {
		if (idx != n)
			merge(idx);
		else
			merge(idx - 1);
	}
}

// 从当前节点的前一个兄弟节点借一个键
// 将父节点中的键下移，并将兄弟节点的最大键移至当前节点以补充不足
void BTreeNode::borrowFromPrev(int idx) {
	BTreeNode *child = children[idx];
	BTreeNode *sibling = children[idx - 1];
	
	// 为当前子节点的键数组腾出位置
	for (int i = child->n - 1; i >= 0; --i)
		child->keys[i + 1] = child->keys[i];
	
	// 如果子节点不是叶节点，移动子节点的所有孩子指针
	if (!child->leaf) {
		for (int i = child->n; i >= 0; --i)
			child->children[i + 1] = child->children[i];
	}
	
	// 将父节点中的键移到子节点
	child->keys[0] = keys[idx - 1];
	
	// 如果子节点不是叶节点，将兄弟节点的最后一个孩子移动到子节点
	if (!child->leaf)
		child->children[0] = sibling->children[sibling->n];
	
	// 将兄弟节点的最后一个键上移到父节点
	keys[idx - 1] = sibling->keys[sibling->n - 1];
	
	child->n += 1;
	sibling->n -= 1;
}

// 从当前节点的下一个兄弟节点借一个键
// 将父节点中的键下移，并将兄弟节点的最小键移至当前节点以补充不足
void BTreeNode::borrowFromNext(int idx) {
	BTreeNode *child = children[idx];
	BTreeNode *sibling = children[idx + 1];
	
	// 将父节点中的键移到子节点的末尾
	child->keys[child->n] = keys[idx];
	
	// 如果子节点不是叶节点，移动兄弟节点的第一个孩子指针到子节点
	if (!child->leaf)
		child->children[child->n + 1] = sibling->children[0];
	
	// 将兄弟节点的第一个键移到父节点
	keys[idx] = sibling->keys[0];
	
	// 调整兄弟节点的键和孩子指针数组
	for (int i = 1; i < sibling->n; ++i)
		sibling->keys[i - 1] = sibling->keys[i];
	if (!sibling->leaf) {
		for (int i = 1; i <= sibling->n; ++i)
			sibling->children[i - 1] = sibling->children[i];
	}
	
	child->n += 1;
	sibling->n -= 1;
}

// 合并当前节点中的索引为 idx 的子节点与其下一个兄弟节点
// 将父节点中的键和右兄弟节点的键与子节点合并，形成一个包含两个子节点的单一节点
void BTreeNode::merge(int idx) {
	int t = ceil(m / 2.0) - 1;
	BTreeNode *child = children[idx];
	BTreeNode *sibling = children[idx + 1];
	
	// 将父节点中的键移到子节点的中间位置
	child->keys[t] = keys[idx];
	
	// 将兄弟节点的所有键和孩子指针移入子节点
	for (int i = 0; i < sibling->n; ++i)
		child->keys[i + t + 1] = sibling->keys[i];
	if (!child->leaf) {
		for (int i = 0; i <= sibling->n; ++i)
			child->children[i + t + 1] = sibling->children[i];
	}
	
	// 移动父节点的键和子节点指针，填补合并后留下的空位
	for (int i = idx + 1; i < n; ++i)
		keys[i - 1] = keys[i];
	for (int i = idx + 2; i <= n; ++i)
		children[i - 1] = children[i];
	
	// 更新合并后的子节点键数，减少当前节点键数，并释放右兄弟节点的内存
	child->n += sibling->n + 1;
	n--;
	delete sibling;
}


int main() {
	BTree t(16); // 创建一个阶数为4的B树
	for(int i = 1; i <= 200; i++){
		t.insert(Data(i));
		t.traverse();
	}
	
	cout << "遍历B树: ";
	t.traverse();
	cout << endl;
	
	t.remove(6);
	cout << "删除6后的B树遍历: ";
	t.traverse();
	cout << endl;
	
	t.remove(4);
	t.remove(8);
	t.remove(12);
	t.remove(16);
	
	cout << "最终B树遍历: ";
	t.traverse();
	cout << endl;
	
	return 0;
}

