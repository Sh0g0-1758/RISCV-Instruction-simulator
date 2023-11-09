#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>

int main()
{
    using namespace nana;
    form fm;
    fm.caption(L"Hello, World!");
    button btn(fm, rectangle{20, 20, 150, 30});
    btn.caption(L"Quit");
    btn.events().click(API::exit);
    fm.show();
    exec();
}