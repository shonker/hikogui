//
//  Window.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once
#include <memory>
#include <unordered_set>
#include <boost/thread.hpp>
#include <vulkan/vulkan.hpp>
#include "Rectangle.hpp"
#include "View.hpp"
#include "BackingCache.hpp"
#include "BackingPipeline.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Instance;
class Device;


enum class WindowType {
    WINDOW,
    PANEL,
    FULLSCREEN,
};

enum class SubpixelLayout {
    NONE,
    RGB_LEFT_TO_RIGHT,
    RGB_RIGHT_TO_LEFT,
    RGB_TOP_TO_BOTTOM,
    RGB_BOTTOM_TO_TOP,
};

enum class WindowState {
    NO_DEVICE,
    LINKED_TO_DEVICE,
    READY_TO_DRAW,
};

struct WindowStateError: virtual boost::exception, virtual std::exception {};

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI Toolkit, because
 * modern design requires drawing of user interface elements in the border.
 */
class Window {
private:
    boost::shared_mutex stateMutex;
    WindowState state;

public:
    vk::SurfaceKHR intrinsic;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    vk::SwapchainCreateInfoKHR swapchainCreateInfo;

    vk::SwapchainKHR swapchain;

    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;
    std::vector<vk::CommandBuffer> swapchainCommandBuffers;

    vk::RenderPass firstRenderPass;
    vk::RenderPass followUpRenderPass;

    Instance *instance;
    Device *device;

    vk::Rect2D displayRectangle;

    //! Location of the window on the screen.
    Rectangle location;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi;

    /*! Pixels-per-Point
     * A point references a typefraphic point, 1/72 inch.
     * Scale all drawing and sizing on the window using this attribute.
     * This value is rounded to an integer value for drawing clean lines.
     */
    float ppp;

    /*! Definition on how subpixels are oriented on the window.
     * If the window is located on multiple screens with different pixel layout then
     * `SubpixelLayout::NONE` should be selected.
     */
    SubpixelLayout subpixelLayout;

    //! The view covering the complete window.
    std::shared_ptr<View> view;

    /*! Type of window.
     * The type of window dictates the way the window-decoration and possibly the
     * rest of the user interface is drawn. This may switch during execution
     * for example switching to `FULLSCREEN` and back to `WINDOW`.
     */
    WindowType windowType;

    /*! A set of backings.
     */
    BackingCache backings;

    std::shared_ptr<BackingPipeline> backingPipeline;

    void buildSwapchainAndPipeline(void);
    void teardownSwapchainAndPipeline(void);

    void rebuildSwapchainAndPipeline(void) {
        teardownSwapchainAndPipeline();
        buildSwapchainAndPipeline();
    }

    void setDevice(Device *device);

    /*! Refresh Display.
     *
     * \outTimestamp Number of nanoseconds since system start.
     * \outputTimestamp Number of nanoseconds since system start until the frame will be displayed on the screen.
     */
    void frameUpdate(uint64_t nowTimestamp, uint64_t outputTimestamp);

    Window(Instance *instance, vk::SurfaceKHR surface) :
        state(WindowState::NO_DEVICE), instance(instance), intrinsic(surface)
    {

    }

    // PipelineDestination

    virtual Device *getPipelineDestinationDevice(void) const {
        return device;
    }

    virtual vk::Extent2D getPipelineDestinationImageExtent(void) const {
        return swapchainCreateInfo.imageExtent;
    }

    virtual vk::Format getPipelineDestinationImageFormat(void) const {
        return swapchainCreateInfo.imageFormat;
    }

private:
    void buildSwapchain(void);
    void teardownSwapchain(void);
    void buildRenderPasses(void);
    void teardownRenderPasses(void);
    void buildFramebuffers(void);
    void teardownFramebuffers(void);
    void buildPipelines(void);
    void teardownPipelines(void);
};

}}}
