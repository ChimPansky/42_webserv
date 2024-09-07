LocationBlock::LocationBlock(const std::vector<S>& settings)
    : allowed_methods_(),
      redirect_(301, ""),
      is_cgi_(false),
      root_dir_(""),
      default_file_(""),
      dir_listing_("")
{
    allowed_methods_.push_back("GET");
    allowed_methods_.push_back("POST");
    // parse values from the map
    (void)settings;
}

const std::string& LocationBlock::route() const
{
    return route_;
}

const std::vector<std::string>& LocationBlock::allowed_methods() const
{
    return allowed_methods_;
}

const std::pair<int, std::string>& LocationBlock::redirect() const
{
    return redirect_;
}

bool LocationBlock::is_cgi() const
{
    return is_cgi_;
}

const std::vector<std::string>& LocationBlock::cgi_paths() const
{
    return cgi_paths_;
}

const std::vector<std::string>& LocationBlock::cgi_extensions() const
{
    return cgi_extensions_;
}

const std::string& LocationBlock::root_dir()
{
    return root_dir_;
}

const std::string& LocationBlock::default_file() const
{
    return default_file_;
}

const std::string& LocationBlock::dir_listing() const
{
    return dir_listing_;
}