// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "FileView.hpp"
#include "StaticResourceView.hpp"
#include "Application.hpp"
#include "logger.hpp"
#include <variant>

namespace TTauri {

using ResourceView_intrinsic = std::variant<FileView,StaticResourceView>;

static inline ResourceView_intrinsic loadView(URL const &location)
{
    if (location.scheme() == "resource") {
        try {
            let view = StaticResourceView(location.filename());
            LOG_INFO("Loaded resource {} from executable.", location);
            return view;
        } catch (key_error) {
            let absoluteLocation = URL::urlFromResourceDirectory() / location;
            auto view = FileView{ absoluteLocation };
            LOG_INFO("Loaded resource {} from filesystem at {}.", location, absoluteLocation);
            return view;
        }

    } else if (location.scheme() == "file") {
        if (!location.isAbsolute()) {
            TTAURI_THROW(url_error("file-URLs must be absolute.")
                .set<"url"_tag>(location)
            );
        }

        auto view = FileView{ location };
        LOG_INFO("Loaded resource {} from filesystem.", location);
        return view;

    } else {
        TTAURI_THROW(url_error("Unknown scheme for loading a resource")
            .set<"url"_tag>(location)
        );
    }
}

class ResourceView {
private:
    ResourceView_intrinsic intrinsic;

public:
    ResourceView(URL const &location) : intrinsic(loadView(location)) {}

    ResourceView() = delete;
    ~ResourceView() = default;
    ResourceView(ResourceView const &other) = default;
    ResourceView(ResourceView &&other) = default;
    ResourceView &operator=(ResourceView const &other) = default;
    ResourceView &operator=(ResourceView &&other) = default;

    size_t offset() const noexcept {
        if (std::holds_alternative<FileView>(intrinsic)) {
            let &_view = std::get<FileView>(intrinsic);
            return _view.offset;
        } else {
            return 0;
        }
    }

    gsl::span<std::byte const> bytes() const noexcept {
        if (std::holds_alternative<FileView>(intrinsic)) {
            let &_intrinsic = std::get<FileView>(intrinsic);
            return _intrinsic.bytes();
        } else if (std::holds_alternative<StaticResourceView>(intrinsic)) {
            let &_intrinsic = std::get<StaticResourceView>(intrinsic);
            return _intrinsic.bytes();
        } else {
            no_default;
        }
    }

    /*
    gsl::span<std::byte> writableBytes() noexcept {
        if (std::holds_alternative<FileView>(intrinsic)) {
            let &_intrinsic = std::get<FileView>(intrinsic);
            return _intrinsic.bytes();
        } else {
            no_default;
        }
    }*/

    size_t size() const noexcept {
        return bytes().size();
    }

    void const *data() const noexcept {
        return bytes().data();
    }
};

}