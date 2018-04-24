/*
 * Copyright 2017, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "i_shell.h"

Output<char> IShell::ExecuteCommand(const std::string &cmd) {
#ifdef _WIN32
  std::string command = "PowerShell -Command " + cmd + " 2>&1";
#else
  std::string command =
      "{ " + (address_.empty()
                  ? ""
                  : "ssh -o PasswordAuthentication=no " + address_ + " ") +
      cmd + "; } 2>&1";
#endif  // _WIN32
  std::unique_ptr<FILE, PipeDeleter> pipe(popen(command.c_str(), "r"));

  if (!pipe) {
    throw std::runtime_error("popen failed");
  }

  char buffer[BUFFER_SIZE];
  std::string out_buffer;

  while (fgets(buffer, BUFFER_SIZE, pipe.get())) {
    out_buffer.append(buffer);
  }

  auto s_pipe = pipe.release();
  int exit_code = pclose(s_pipe);
#ifdef _WIN32
  output_ = Output<char>(exit_code, out_buffer);
#else
  output_ = Output<char>(WEXITSTATUS(exit_code), out_buffer);
#endif  // _WIN32

  if (print_log_) {
    std::cout << out_buffer << std::endl;
  }

  return output_;
}
