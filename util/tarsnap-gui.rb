class TarsnapGui < Formula
  desc "Cross-platform GUI for the Tarsnap command-line client"
  homepage "https://github.com/Tarsnap/tarsnap-gui/wiki/Tarsnap"
  url "https://github.com/Tarsnap/tarsnap-gui/archive/v1.0.2.tar.gz"
  sha256 "3b271f474abc0bbeb3d5d62ee76b82785c7d64145e6e8b51fa7907b724c83eae"
  head "https://github.com/Tarsnap/tarsnap-gui.git"

  depends_on "qt"
  depends_on "tarsnap"

  def install
    system "qmake"
    system "make"
    system "macdeployqt", "Tarsnap.app"
    prefix.install "Tarsnap.app"
  end

  def post_install
    system "#{opt_prefix}/Tarsnap.app/Contents/MacOS/Tarsnap", "--check"
  end

  test do
    system "#{opt_prefix}/Tarsnap.app/Contents/MacOS/Tarsnap", "--version"
  end
end
