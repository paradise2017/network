#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;
class Solution
{
public:
    vector<int> twoSum(vector<int> nums, int target)
    {
        unordered_map<int, int> result;

        for (int i = 0; i < nums.size(); i++)
        {
            int cur_target = target - nums[i];
            if (result.find(cur_target) != result.end())
            {
                return {result[cur_target], i};
            }
            result[nums[i]] = i;
        }
        return {}; // Return an empty vector if no solution is found
    };
};

std::string to_string1(int num)
{
    return std::to_string(num);
}
int main()
{
    // Solution s;                                       // test
    // vector<int> iter1 = s.twoSum({2, 7, 11, 15}, 13); // test

    // for (auto &iter : iter1)
    // {
    //     cout << iter << " " << endl;
    // }
    // cout << to_string1(iter1[0]) << endl;

    const unsigned char MESSAGE_SIZE = 7;
    char buf[MESSAGE_SIZE];
    cout << sizeof(buf)<< endl;
    return 0;
}