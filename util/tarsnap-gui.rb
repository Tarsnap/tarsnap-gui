class TarsnapGui < Formula
  desc "Cross-platform GUI front-end for the Tarsnap command-line client."
  homepage "https://github.com/Tarsnap/tarsnap-gui/wiki/Tarsnap"
  url "https://github.com/Tarsnap/tarsnap-gui/archive/v1.0.tar.gz"
  sha256 ""
  head "https://github.com/Tarsnap/tarsnap-gui.git"

  depends_on "qt5"
  depends_on "tarsnap"

  def install
    system "qmake"
    system "make"
    system "macdeployqt Tarsnap.app"
    prefix.install "Tarsnap.app"
  end

  test do
    system "#{opt_prefix}/Tarsnap.app/Contents/MacOS/Tarsnap", "--version"
  end
end
