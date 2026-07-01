#ifndef CONTAINER_TPP
#define CONTAINER_TPP

namespace UI {

template <typename T, typename... Args>
T* Container::createChild(Args&&... args) {
    auto child = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = child.get();

    auto activeOpt = activeChildDefaults();
    if (activeOpt.has_value()) {
        const auto& defaults = *activeOpt;
        if (defaults.modeX.has_value()) {
            ptr->setModeX(*defaults.modeX);
        }
        if (defaults.modeY.has_value()) {
            ptr->setModeY(*defaults.modeY);
        }
        if (defaults.fixedWidth.has_value()) {
            ptr->setFixedWidth(*defaults.fixedWidth);
        }
        if (defaults.fixedHeight.has_value()) {
            ptr->setFixedHeight(*defaults.fixedHeight);
        }
        if (defaults.margin.has_value()) {
            ptr->setMargins(defaults.margin->top, defaults.margin->bottom, defaults.margin->left, defaults.margin->right);
        }
    }

    addChild(std::move(child));
    return ptr;
}

} // namespace UI

#endif // CONTAINER_TPP
