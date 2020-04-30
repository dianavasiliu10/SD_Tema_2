#include <iostream>
#include <list>
#include <fstream>
#include <vector>

using namespace std;

struct Node
{
    int grad, info;
    Node* parinte;
    Node* frate;
    Node* copil;
};

list<Node*> heap;
vector<int> toDelete;

Node* newNode(int key)
{
    Node *temp = new Node;
    temp->info = key;
    temp->grad = 0;
    temp->copil = temp->parinte = temp->frate = nullptr;
    return temp;
}

Node* mergeBTrees(Node* b1, Node* b2)
{
    if (b1->info > b2->info) {      //b1 trebuie sa fie mai mic decat b2
        Node* aux = b1;
        b1=b2;
        b2=aux;
    }

    b2->parinte = b1;
    b2->frate = b1->copil;
    b1->copil = b2;
    b1->grad++;         // arborele cu radacina mai mare devine copilul arborelui cu radacina mai mica

    return b1;
}

list<Node*> Union(list<Node*> &h1, list<Node*> &h2)
{
    list<Node *> nnew;      // va contine h1 si h2 dupa merge
    auto it1 = h1.begin();
    auto it2 = h2.begin();
    while (it1 != h1.end() && it2 != h2.end())
    {
        if ((*it1)->grad <= (*it2)->grad)
        {
            nnew.push_back(*it1);
            it1++;
        }
        else
        {
            nnew.push_back(*it2);
            it2++;
        }
    }

    // se adauga elementele ramase din h1
    while (it1 != h1.end())
    {
        nnew.push_back(*it1);
        it1++;
    }

    // de adauga elementele ramase din h2
    while (it2 != h2.end())
    {
        nnew.push_back(*it2);
        it2++;
    }
    return nnew;
}

void extractMin();

// rearanjez heap-ul dupa regula unui heap (tatal < oricare dintre fii)
// si crescator dupa gradul arborilor binomiali din care este format heap-ul
// + se tine cont de faptul ca nu exista 2 arbori binomiali de acelasi grad
// in cazul in care exista, ii uneste
void adjust()
{
    if (heap.size() > 1) {
        list<Node*> newheap;
        auto it1=heap.begin();
        auto it2=heap.begin();
        auto it3=heap.begin();

        //it1 < it2 < it3
        if (heap.size() == 2) {
            it2 = it1;
            it2++;
            it3 = heap.end();
        }
        else {
            it2++;
            it3 = it2;
            it3++;
        }

        while (it1 != heap.end())
        {
            if (it2 == heap.end())      // daca a ramas un singur element de verificat
                it1++;

            //daca grad(it1) < grad(it2), adica nu putem apela merge(), atunci ma mut in urmatorii arbori binomiali
            else if ((*it1)->grad < (*it2)->grad) {
                it1++;
                it2++;
                if (it3 != heap.end())
                    it3++;
            }

            // daca grad(it1)==grad(it2)==grad(it3),
            else if (it3 != heap.end() &&
                     (*it1)->grad == (*it2)->grad &&
                     (*it1)->grad == (*it3)->grad)
            {
                it1++;
                it2++;
                it3++;
            }

            // daca grad(it1)==grad(it2), atunci apelez merge() pe celi 2 arbori
            else if ((*it1)->grad == (*it2)->grad) {
                *it1 = mergeBTrees(*it1, *it2);
                it2 = heap.erase(it2);
                if (it3 != heap.end())
                    it3++;
            }
        }
    }
}

void insertTree(Node* tree)
{
    list<Node*> temp;
    temp.push_back(tree);       // creez un arbore temporar (arborele pe care vreau sa il adaug)
    heap = Union(heap, temp);      // fac reuniunea intre heap-ul original si acest heap temporar

    adjust();
}

void insert(int key)
{
    Node* temp = newNode(key);      //inserarea unei valori: creez un nod avand valoarea key (este echivalent cu un arbore cu un singur nod)
    insertTree(temp);               //il introduc in heap-ul initial ca reuniune de 2 arbori
}

//dintr-un arbore binomial, elimin minimul (radacina) si creez un heap binomial
list<Node*> removeMinTree(Node* tree)
{
    list<Node*> newheap;
    Node* temp = tree->copil;   //temp = primul copil al radacinii
    Node* rad;

    while (temp) {
        rad = temp;
        temp = temp->frate;     //temp trece la urmatorul frate
        rad->frate = nullptr;     // se sterge legatura dintre frati
        newheap.push_front(rad);     // se adauga in noul heap fiecare dintre arborii creati prin ruperea legaturilor dintre frati
    }

    return newheap;
}

Node* getMin()
{
    auto it = heap.begin();
    Node* temp = *it;       // minimul e considerat prima data primul element (prima radacina)
    while (it != heap.end()) {
        if ((*it)->info < temp->info)   //daca se gaseste un element (radacina) mai mic
            temp = *it;         //temp ia valoarea mai mica (nodul cu valoarea mai mica)
        it++;
    }
    return temp;
}

void extractMin()
{
    list<Node*> newheap, newheap2;
    Node* temp;

    temp = getMin();        //heapul cu radacina minima (elementul minim din heap)
    auto it = heap.begin();
    while (it != heap.end()) {
        if (*it != temp) {
            newheap.push_back(*it);     //in newheap se adauga toate elementele din heap-ul initial care NU sunt minime
        }
        it++;
    }
    newheap2 = removeMinTree(temp);     //newheap2 = heap-ul format din arborele ce contine elementul minim, dupa ce acest element a fost eliminat
    heap = Union(newheap, newheap2);    //fac reuniunea dintre heap-ul initial si heap-ul format dupa eliminarea minimului
    adjust();
}

void adjust2();
void lazyDelete(int key)
{
    if (key >= toDelete.size())     // inseamna ca key este cel mai mare element sters de pana acum, deci trebuie sa ii fac loc
        toDelete.resize(key+1);     //ii fac loc in toDelete

    toDelete[key] = 1;

    adjust2();
}

// in momentul in care un element care trebuie sters a ajuns in varf, trebuie eliminat
void adjust2()
{
    int ok = 1;
    while (ok) {        // cat timp elimin noduri
        ok=0;
        auto it = heap.begin();
        while (it != heap.end() && ok==0) {     // parcurg heap-ul (radacinile)
            if (toDelete[(*it)->info] == 1) {       //daca trebuie sters
                toDelete[(*it)->info] = 0;          // il notez ca nu mai trebuie sters
                (*it)->info = -1;           //ma asigur ca are cea mai mica valoare din heap
                extractMin();           // il elimin
                ok=1;
            }
            else
               it++;
        }
    }
}

void printTree(Node* h)
{
    while (h) {     //cat timp mai are frati
        cout << h->info << " ";
        printTree(h->copil);
        h = h->frate;               //trec la urmatorul frate
    }
}

void printHeap()
{
    auto it = heap.begin();
    while (it != heap.end()) {      //afisez fiecare arbore binomial din heap, pe rand
        printTree(*it);
        cout<<"/ ";      // pentru a separa arborii
        it++;
    }
    cout<<"\n";
}

int main()
{
    fstream fin("input.txt");
    int n,op,key;
    Node* minim;
    fin>>n;

    for (int i=0; i<n; i++) {
        fin>>op;
        switch(op) {
            case 1:
                fin>>key;
                insert(key);
//                if (key < toDelete.size())      //daca am inserat o posibila valoare care trebuia stearsa, nu o mai sterg
//                    toDelete[key] = 0;
                break;
            case 2:
                fin>>key;
                lazyDelete(key);
                break;
            case 3:
                minim = getMin();
                cout<<"minim="<<minim->info<<"\n";
                break;
            case 4:
                extractMin();
                break;
            default:
                cout<<"Wrong operation!\n";

        }
    }

    adjust2();

    for (int i=0; i<toDelete.size(); i++)
        if (toDelete[i] == 1)
            cout<<i<<" ";

    cout<<"\n";

    printHeap();

}


