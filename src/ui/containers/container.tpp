#ifndef CONTAINER_TPP
#define CONTAINER_TPP

namespace UI {

template <typename T, typename... Args>
T* Container::createChild(Args&&... args) {
    auto child = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = child.get();
    addChild(std::move(child));
    return ptr;
}

} // namespace UI

#endif // CONTAINER_TPP
