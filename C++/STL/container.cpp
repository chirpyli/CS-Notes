#include<iostream>
#include<vector>
#include<list>
#include<deque>
#include<stack>
#include<queue>
#include<map>
#include<set>
#include<unordered_map>


int main() {
    std::multimap<int, int> m_map;
    std::set<int> m_set;
    std::unordered_map<

    std::vector<int> m_vector {12, 11, 10, 19, 4, 3};
    std::make_heap(std::begin(m_vector), std::end(m_vector));
    if (std::is_heap(std::begin(m_vector), std::end(m_vector))) {
        std::cout << "is heap." << std::endl;
    } else {
        std::cout << "not heap." << std::endl;
    } 

    for (auto i : m_vector) {
        std::cout << i << " ";
    }  
    std::cout << std::endl;
    m_vector.push_back(1);
    std::push_heap(std::begin(m_vector), std::end(m_vector));

    for (auto i : m_vector) {
        std::cout << i << " ";
    } 
    std::cout << std::endl;

    std::sort_heap(std::begin(m_vector), std::end(m_vector));

    for (auto i : m_vector) {
        std::cout << i << " ";
    } 
    std::cout << std::endl;
}