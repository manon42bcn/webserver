/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ws_permissions_bitwise.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 02:11:15 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/14 02:12:18 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WS_PERMISSIONS_BITWISE_HPP
#define WS_PERMISSIONS_BITWISE_HPP

#define MASK_METHOD_GET     (1 << 0)
#define MASK_METHOD_OPTIONS (1 << 1)
#define MASK_METHOD_HEAD    (1 << 2)
#define MASK_METHOD_POST    (1 << 3)
#define MASK_METHOD_PUT     (1 << 4)
#define MASK_METHOD_PATCH   (1 << 5)
#define MASK_METHOD_TRACE   (1 << 6)
#define MASK_METHOD_DELETE  (1 << 7)
// CHECK PRIVILEGES
#define HAS_GET(permissions)     ((permissions) & MASK_METHOD_GET)
#define HAS_OPTIONS(permissions) ((permissions) & MASK_METHOD_OPTIONS)
#define HAS_HEAD(permissions)    ((permissions) & MASK_METHOD_HEAD)
#define HAS_POST(permissions)    ((permissions) & MASK_METHOD_POST)
#define HAS_PUT(permissions)     ((permissions) & MASK_METHOD_PUT)
#define HAS_PATCH(permissions)   ((permissions) & MASK_METHOD_PATCH)
#define HAS_TRACE(permissions)   ((permissions) & MASK_METHOD_TRACE)
#define HAS_DELETE(permissions)  ((permissions) & MASK_METHOD_DELETE)
// GRANT
#define GRANT_GET(permissions)     ((permissions) |= MASK_METHOD_GET)
#define GRANT_OPTIONS(permissions) ((permissions) |= MASK_METHOD_OPTIONS)
#define GRANT_HEAD(permissions)    ((permissions) |= MASK_METHOD_HEAD)
#define GRANT_POST(permissions)    ((permissions) |= MASK_METHOD_POST)
#define GRANT_PUT(permissions)     ((permissions) |= MASK_METHOD_PUT)
#define GRANT_PATCH(permissions)   ((permissions) |= MASK_METHOD_PATCH)
#define GRANT_TRACE(permissions)   ((permissions) |= MASK_METHOD_TRACE)
#define GRANT_DELETE(permissions)  ((permissions) |= MASK_METHOD_DELETE)
// REVOKE
#define REVOKE_GET(permissions)     ((permissions) &= ~MASK_METHOD_GET)
#define REVOKE_OPTIONS(permissions) ((permissions) &= ~MASK_METHOD_OPTIONS)
#define REVOKE_HEAD(permissions)    ((permissions) &= ~MASK_METHOD_HEAD)
#define REVOKE_POST(permissions)    ((permissions) &= ~MASK_METHOD_POST)
#define REVOKE_PUT(permissions)     ((permissions) &= ~MASK_METHOD_PUT)
#define REVOKE_PATCH(permissions)   ((permissions) &= ~MASK_METHOD_PATCH)
#define REVOKE_TRACE(permissions)   ((permissions) &= ~MASK_METHOD_TRACE)
#define REVOKE_DELETE(permissions)  ((permissions) &= ~MASK_METHOD_DELETE)
// MULTI FLAGS
#define HAS_PERMISSION(permissions, mask)     ((permissions) & MASK_METHOD_GET)
#define GRANT_PERMISSIONS(permissions, mask)  ((permissions) |= (mask))
#define REVOKE_PERMISSIONS(permissions, mask) ((permissions) &= ~(mask))

#endif