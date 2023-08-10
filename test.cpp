#include <cstddef>
#include <vector>
#include <limits>
#include <iostream>

template<class T>
T max(const std::vector<T>& arr){
    T mx = std::numeric_limits<T>::min();
    for (int i=0;i<arr.size();i++){
        mx = std::max(mx, arr[i]);
    }
    return mx;
}

template<class T>
class shared_ptr{
    public:
        int *ref_count;
        T *ptr;
        shared_ptr(const T& obj);
        shared_ptr(const shared_ptr& other);
        shared_ptr& operator=(const shared_ptr& other);

        T* operator->() const {
            return ptr;
        }
        T& operator*() const {
            return *ptr;
        }

        int get_ref_count() const { return *ref_count; }
        ~shared_ptr();
};

template<class T>
shared_ptr<T> :: shared_ptr(const T& obj){
    ref_count = new int(1);
    ptr = new T(obj);
}

template<class T>
shared_ptr<T> :: shared_ptr(const shared_ptr& other){
    ref_count = other.ref_count;
    (*ref_count)++;
    ptr = other.ptr;
}

template<class T>
shared_ptr<T>& shared_ptr<T> :: operator=(const shared_ptr& other){
    ref_count = other.ref_count;
    (*ref_count)++;
    ptr = other.ptr;

    return *this;
}

template<class T>
shared_ptr<T> :: ~shared_ptr(){
    std::cout << "Destructor called!\n";
    if (ref_count && ptr){
        (*ref_count)--;
        if (!(*ref_count)){
            delete ref_count;
            delete ptr;
            std::cout << "Deleted data!\n";
        }
    }
}

struct A{
    int x;
    double y;
};

int main(){
    // std::vector<int> arr1 = {1, 2, 10, 5};
    // std::vector<double> arr2 = {3.1251, 0.15, 12.2};
    // std::cout << max(arr1) << "\n" << max(arr2) << "\n";
    {
        shared_ptr<int> ptr1 = {4};
        std::cout << *ptr1 << "\n";
        {
            shared_ptr<int> ptr2 = {ptr1};
            std::cout << *ptr2 << "\n";
        }
    }
    {
        shared_ptr<A> ptr1 = A{2, 21.214};
        std::cout << "x : " << ptr1->x << "\n";
        std::cout << "y : " << ptr1->y << "\n";
        {
            shared_ptr<A> ptr2 = {ptr1};
            std::cout << "x : " << ptr2->x << "\n";
            std::cout << "y : " << ptr2->y << "\n";
            ptr2->x = 4;
            ptr2->y = 125.15;
        }
        std::cout << "x : " << ptr1->x << "\n";
        std::cout << "y : " << ptr1->y << "\n";
    }
    {
        shared_ptr<std::vector<int>> ptr1 = std::vector<int>{2, 4, 5};
        ptr1->push_back(6);
        {
            shared_ptr<std::vector<int>> ptr2 = {ptr1};
            for (const auto& x : *ptr2){
                std::cout << x << " ";
            }
            std::cout << "\n";
            ptr2->pop_back();
            ptr2->push_back(7);
        }
        for (const auto& x : *ptr1){
            std::cout << x << " ";
        }
        std::cout << "\n";
    }
}