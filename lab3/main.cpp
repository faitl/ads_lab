#include <iostream>
#include <vector>
#include <random>
#include <fstream>

struct stats {
    size_t comparison_count = 0;
    size_t copy_count = 0;
};

static void swap_int(int& a, int& b, stats& s) {
    int temp = a; s.copy_count++;
    a = b; s.copy_count++;
    b = temp; s.copy_count++;
}

// пузырьковая на списке
class LinkedList {
private:
    struct Node {
        int value;
        Node* next;
        Node(int v) : value(v), next(nullptr) {}
    };

    Node* head_;

    void clear() {
        Node* p = head_;
        while (p) {
            Node* n = p->next;
            delete p;
            p = n;
        }
        head_ = nullptr;
    }


};
public:
    LinkedList() : head_(nullptr) {}
    ~LinkedList() { clear(); }

    void from_vector(const std::vector<int>& v) {
        clear();
        if (v.empty()) return;
        head_ = new Node(v[0]);
        Node* tail = head_;
        for (size_t i = 1; i < v.size(); ++i) {
            tail->next = new Node(v[i]);
            tail = tail->next;
        }
    }

    // пузырек
    stats bubble_sort() {
        stats s;
        if (!head_ || !head_->next)
            return s;

        bool swapped = true;
        while (swapped) {
            swapped = false;
            Node* p = head_;
            while (p->next != nullptr) {
                s.comparison_count++;
                if (p->value > p->next->value) {
                    int temp = p->value; s.copy_count++;
                    p->value = p->next->value; s.copy_count++;
                    p->next->value = temp; s.copy_count++;
                    swapped = true;
                }
                p = p->next;
            }
        }
        return s;
    }




// шейкерная
stats shaker_sort(std::vector<int>& arr) {
    stats s;
    if (arr.size() < 2) return s;

    size_t left = 0;
    size_t right = arr.size() - 1;
    bool swapped = true;

    while (swapped) {
        swapped = false;

        for (size_t i = left; i < right; ++i) {
            s.comparison_count++;
            if (arr[i] > arr[i + 1]) {
                swap_int(arr[i], arr[i + 1], s);
                swapped = true;
            }
        }
        if (!swapped) break;
        right--;

        swapped = false;

        for (size_t i = right; i > left; --i) {
            s.comparison_count++;
            if (arr[i - 1] > arr[i]) {
                swap_int(arr[i - 1], arr[i], s);
                swapped = true;
            }
        }
        left++;
    }

    return s;
}


// быстрая
static int partition_hoare(std::vector<int>& arr, int low, int high, stats& s) {
    int pivot = arr[(low + high) / 2];
    s.copy_count++;

    int i = low - 1;
    int j = high + 1;

    while (true) {
        do {
            i++;
            s.comparison_count++;
        } while (arr[i] < pivot);

        do {
            j--;
            s.comparison_count++;
        } while (arr[j] > pivot);

        if (i >= j)
            return j;

        swap_int(arr[i], arr[j], s);
    }
}

static void quick_sort_impl(std::vector<int>& arr, int low, int high, stats& s) {
    if (low < high) {
        int p = partition_hoare(arr, low, high, s);
        quick_sort_impl(arr, low, p, s);
        quick_sort_impl(arr, p + 1, high, s);
    }
}

stats quick_sort(std::vector<int>& arr) {
    stats s;
    if (arr.size() < 2) return s;
    quick_sort_impl(arr, 0, static_cast<int>(arr.size() - 1), s);
    return s;
}


//вспомогательное
std::vector<int> gen_random(size_t n, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, 1000000);
    std::vector<int> v(n);
    for (size_t i = 0; i < n; ++i)
        v[i] = dist(rng);
    return v;
}

void write_header(std::ofstream& f) {
    f << "n,comparison_count,copy_count\n";
}

int main() {
    const std::vector<size_t> sizes =
    { 1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,25000,50000,100000 };

    const int trials = 100;
    const unsigned seed = 123456u;
    std::mt19937 rng(seed);

    std::ofstream fout_random("results_random.csv");
    std::ofstream fout_sorted("results_sorted.csv");
    std::ofstream fout_reverse("results_reverse.csv");

    write_header(fout_random);
    write_header(fout_sorted);
    write_header(fout_reverse);

    const unsigned long long MAX_OPS = 500000000ULL;

    for (size_t n : sizes) {
        std::cout << "processing n = " << n << "\n";

        unsigned long long est = (unsigned long long)n * (unsigned long long)n;
        bool skip = (est > MAX_OPS);

        if (skip) {
            std::cout << "too large (skipped)\n";
            fout_random << n << ",-1,-1\n";
            fout_sorted << n << ",-1,-1\n";
            fout_reverse << n << ",-1,-1\n";
            continue;
        }

        // рандомный
        unsigned long long cmp = 0, cpy = 0;
        for (int t = 0; t < trials; ++t) {
            auto v = gen_random(n, rng);
            //LinkedList lst; lst.from_vector(v); stats s = lst.bubble_sort();
            //stats s = shaker_sort(v);
            stats s = quick_sort(v);
            cmp += s.comparison_count;
            cpy += s.copy_count;
        }
        fout_random << n << "," << cmp / trials << "," << cpy / trials << "\n";

        // сортированный
        {
            std::vector<int> v(n);
            for (size_t i = 0; i < n; ++i) v[i] = static_cast<int>(i);

            //LinkedList lst; lst.from_vector(v); stats s = lst.bubble_sort();
            //stats s = shaker_sort(v);
            stats s = quick_sort(v);
            fout_sorted << n << "," << s.comparison_count << "," << s.copy_count << "\n";
        }

        // обратно отсортированный
        {
            std::vector<int> v(n);
            for (size_t i = 0; i < n; ++i) v[i] = static_cast<int>(n - i);

            //LinkedList lst; lst.from_vector(v); stats s = lst.bubble_sort();
            //stats s = shaker_sort(v);
            stats s = quick_sort(v);
            fout_reverse << n << "," << s.comparison_count << "," << s.copy_count << "\n";
        }
    }

    std::cout << "generated: results_random.csv, results_sorted.csv, results_reverse.csv\n";

    return 0;
}
