/*
 * Copyright (C) 2012 Andre Chen and contributors.
 * andre.hl.chen@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef YUV_TO_RGB
#define YUV_TO_RGB

bool nv12_to_rgb (unsigned char* rgb,
                  unsigned char const* nv12,
                  const int width,
                  const int height);

bool nv12_to_rgba (unsigned char* rgba,
                   unsigned char alpha,
                   unsigned char const* nv12,
                   const int width,
                   const int height);

bool nv21_to_rgb (unsigned char* rgb,
                  unsigned char const* nv21,
                  const int width,
                  const int height);

bool nv21_to_rgba (unsigned char* rgba,
                   unsigned char alpha,
                   unsigned char const* nv21,
                   const int width,
                   const int height);

#endif
