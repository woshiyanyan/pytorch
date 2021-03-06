#include <ATen/ATen.h>
#include <ATen/Parallel.h>
#include <test/cpp/jit/test_base.h>
#include <thread>


// This checks whether threads can see the global
// numbers of threads set and also whether the scheduler
// will throw an exception when multiple threads call
// their first parallel construct.
void test(int given_num_threads) {
  at::init_num_threads();
  auto t = at::ones({1000 * 1000}, at::CPU(at::kFloat));
  ASSERT_TRUE(given_num_threads >= 0);
  ASSERT_EQ(at::get_num_threads(), given_num_threads);
  auto t_sum = t.sum();
  for (int i = 0; i < 1000; ++i) {
    t_sum = t_sum + t.sum();
  }
}

int main() {
  at::init_num_threads();
  at::manual_seed(123);

  test(at::get_num_threads());
  std::thread t1(test, at::get_num_threads());
  t1.join();

  at::set_num_threads(4);
  std::thread t2(test, at::get_num_threads());
  std::thread t3(test, at::get_num_threads());
  std::thread t4(test, at::get_num_threads());
  t4.join();
  t3.join();
  t2.join();

  at::set_num_threads(5);
  test(at::get_num_threads());

  // test inter-op settings
  ASSERT_EQ(at::get_num_interop_threads(), std::thread::hardware_concurrency());
  at::set_num_interop_threads(5);
  ASSERT_EQ(at::get_num_interop_threads(), 5);
  ASSERT_ANY_THROW(at::set_num_interop_threads(6));

  return 0;
}
