#include "type.hpp"
#include <iostream>

namespace Linh
{
    void type(LiVM &vm)
    {
        // Truy cập biến private: cần friend hoặc public getter, hoặc chuyển code này vào LiVM nếu cần.
        // Ở đây giả sử type() là thành viên LiVM, nên chuyển lại thành:
        vm.type();
    }
}
