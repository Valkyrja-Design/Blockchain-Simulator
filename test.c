struct B{
    int x;
};

struct A{
    struct A* a[10];
    struct A* b;
    struct B c;
    int x;
};

struct A foo(){
    struct A a;

    int goo(){

    }
    goo();
    return a;
    goo();
}

int main(){
    struct A a;
    foo().b[0] = a;
    foo().c = a;
    foo().x = 0;
    foo().a[0] = 0;
    (foo().a[0])->x = 0;
}