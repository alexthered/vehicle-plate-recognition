#include "common.h"

cv::Rect ClipRect(cv::Rect rect, cv::Size img_size)
{
    cv::Rect clipped_rect;
    clipped_rect.x = (rect.x < 0) ? 0 : rect.x;
    clipped_rect.y = (rect.y < 0) ? 0 : rect.y;

    clipped_rect.width = ((clipped_rect.x + rect.width) <= img_size.width) ?
                          rect.width : (img_size.width - clipped_rect.x);
    clipped_rect.height = ((clipped_rect.y + rect.height) <= img_size.height) ?
                          rect.height : (img_size.height - clipped_rect.y);

    return clipped_rect;
}
