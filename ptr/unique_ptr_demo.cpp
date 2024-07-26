#include<iostream>
#include<fstream>
#include<locale>
#include<cstdio>
#include<cassert>
#include<memory>
#include<stdexcept>

class B {
    public:
        virtual ~B() = default;
        virtual void bar() { std::cout << "B::bar()" << std::endl;}
};

class D : public B {
    public:
        D() {std::cout << "D::D" << std::endl;}
        ~D() {std::cout << "D::~D" << std::endl;}
        void bar() override {std::cout << "D::bar()" << std::endl;}
};

std::unique_ptr<D> pass_through(std::unique_ptr<D> p){
    p->bar();
    return p;
}

void closer(std::FILE* p){
    std::fclose(p);
}

class List {
    public:
        struct Node {
            int val;
            std::unique_ptr<Node> next;
        };

        std::unique_ptr<Node> head;

        ~List(){
            while(head){
                std::unique_ptr<Node>&& next = std::move(head->next);
                head = std::move(next);

            }
        }

        void push(int data){
            head = std::unique_ptr<Node>(new Node{data, std::move(head)});
        }
};

int main() {
    std::cout << "1)独占所有权语义演示" << std::endl;
    {
        std::unique_ptr<D> p = std::make_unique<D>();
        std::unique_ptr<B> q = pass_through(std::move(p));
        assert(!p);
    }
    
    std::cout << "2)运行时多态演示" << std::endl;
    {
        std::unique_ptr<B> p = std::make_unique<D>();
        p->bar();
    }

    std::cout << "3)自定义删除器演示" << std::endl;
    std::ofstream("demo.txt") << "x";  // 准备读取的文件
    {
        using unique_file_t = std::unique_ptr<std::FILE, decltype(closer)*>;
        unique_file_t fp(std::fopen("demo.txt", "r"), closer);
        if(fp)
            std::cout << char(std::fgetc(fp.get())) << std::endl;
    }

    std::cout << "4) 自定义lambda表达式删除器和异常安全性演示" << std::endl;
    try {
        std::unique_ptr<D, void(*)(D*)> p(new D, [](D *ptr){
            std::cout << "由自定义删除器销毁" << std::endl;
            delete ptr;
        });
        throw std::runtime_error("");
    }catch (const std::exception& e){
        std::cout << "捕获到异常" << std::endl;
    }

    std::cout << "5) 数组形式的unique_ptr演示" << std::endl;
    {
        std::unique_ptr<D[]> p(new D[3]);
    }

    std::cout << "6) 链表演示" << std::endl;
    {
        List wall;
        const int enough{1000000};
        for (int beer = 0; beer != enough; ++beer){
            wall.push(beer);
        }

        std::cout.imbue(std::locale("POSIX"));
        std::cout << "墙上有 " << enough << " 啤酒瓶。。。" << std::endl;
    }

    return 0;
}
