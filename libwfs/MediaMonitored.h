#pragma once

#include <macgyver/StringConversion.h>
#include <memory>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class MediaMonitored
{
 public:
  using MediaValueType = std::string;
  using KeywordType = std::string;
  using MediaValueSetType = std::set<MediaValueType>;
  using KeywordToMediaValueMapType = std::map<KeywordType, MediaValueType>;

  MediaMonitored() = default;
  virtual ~MediaMonitored() = default;
  void add(const KeywordType& keyword)
  {
    // Validate the keyword and insert the MediaValue.
    auto ktmvmIt =
        m_keywordToMediaValueMap.find(Fmi::ascii_tolower_copy(keyword));
    if (ktmvmIt != m_keywordToMediaValueMap.end())
      m_mediaValueSet.insert(ktmvmIt->second);
  }

  MediaValueSetType::const_iterator begin() const { return m_mediaValueSet.begin(); }
  MediaValueSetType::const_iterator end() const { return m_mediaValueSet.end(); }

 private:
  MediaMonitored& operator=(const MediaMonitored& other) = delete;
  MediaMonitored(const MediaMonitored& other) = delete;

  MediaValueSetType m_mediaValueSet;

  // first: keyword, second: meadiValue
  const KeywordToMediaValueMapType m_keywordToMediaValueMap{{"atmosphere", "air"},
                                                            {"surface", "air"},
                                                            {"seasurf", "water"},
                                                            {"sealayer", "water"},
                                                            {"magnetosphere", "air"},
                                                            {"air", "air"},
                                                            {"biota", "biota"},
                                                            {"landscape", "landscape"},
                                                            {"sediment", "sediment"},
                                                            {"soil", "soil"},
                                                            {"ground", "ground"},
                                                            {"waste", "waste"},
                                                            {"water", "water"}};
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
