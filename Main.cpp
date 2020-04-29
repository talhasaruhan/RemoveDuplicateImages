#include <cstdio>
#include <iostream>
#include <bitset>
#include <vector>
#include <filesystem>
#include <algorithm>

#include "Image.h"
#include "DHash.h"

// #define ROOT_DIR "images"
#define EXT_FILTERS {".png", ".jpg", ".jpeg"}

namespace fs = std::filesystem;

typedef std::vector<fs::path> DirList;
typedef std::vector<std::string> ExtFilters;

DirList ListDir(const char* root_dir, const ExtFilters& filters)
{
	DirList dir_list;
	for (const auto& entry : fs::directory_iterator(root_dir))
	{
		fs::path path = entry.path();
		std::string ext = path.extension().u8string();

		if (!fs::is_regular_file(path))
		{
			std::cout << "Skipping directory " << path << "\n";
			continue;
		}

		for (const std::string& filter : filters)
		{
			if (ext == filter)
			{
				dir_list.push_back(path);
				break;
			}
		}
	}
	return dir_list;
}

std::vector<int> FindDuplicates(const DirList& dir_list)
{
	std::vector<uint64_t> hash_list;

	int nimgs = dir_list.size();
	hash_list.resize(nimgs);

	std::cout << "Number of entries: " << nimgs << "\n";

	for (int i = 0; i < nimgs; ++i)
	{
		const auto& path = dir_list[i];
		Image image = ReadImage(path.u8string().c_str());
		if (image.data)
		{
			hash_list[i] = DHash(image);
			stbi_image_free(image.data);
		}
		else
		{
			hash_list[i] = UINT64_MAX;
		}
		if (!(i % 250))
		{
			std::cout << "i = " << i << "\n";
		}
	}
	std::cout << "Finished reading all entries, checking for duplicates!\n";

	std::vector<int> sorted_idx(nimgs);
	std::iota(sorted_idx.begin(), sorted_idx.end(), 0);
	std::sort(sorted_idx.begin(), sorted_idx.end(),
		[&](int a, int b) { return hash_list[a] < hash_list[b]; });

	std::vector<int> duplicate_images;

	int start_range = -1;
	int end_range = -1;
	for (int i = 1; i < nimgs; ++i)
	{
		int ind = sorted_idx[i];
		int prev_ind = sorted_idx[i-1];

		if (hash_list[ind] == hash_list[prev_ind])
		{
			if (start_range < 0) 
			{
				start_range = i - 1;
			}
			end_range = i;
		}

		if (i == nimgs - 1 || hash_list[ind] != hash_list[prev_ind])
		{
			if (start_range >= 0 && end_range >= 0 && start_range != end_range)
			{
				// Load all the images within the range
				std::vector<Image> range_images(end_range-start_range+1);
				for (int j = start_range; j <= end_range; ++j)
				{
					Image image = ReadImage(dir_list[sorted_idx[j]].u8string().c_str());
					image.img_name = dir_list[sorted_idx[j]].u8string();
					range_images[j-start_range] = image;
				}
				std::vector<int> range_idx(end_range-start_range+1);
				std::iota(range_idx.begin(), range_idx.end(), 0);
				std::sort(range_idx.begin(), range_idx.end(), [&](int a, int b) { 
						const Image& img_a = range_images[a];
						const Image& img_b = range_images[b];
						if (img_a.width != img_b.width)
						{
							return img_a.width < img_b.width;
						}
						if (img_a.height != img_b.height)
						{
							return img_a.height < img_b.height;
						}
						return img_a.nchannels < img_b.nchannels;
					});
				for (int j = 1; j < end_range-start_range+1; ++j)
				{
					// First check the dimensions
					const Image& img_1 = range_images[range_idx[j - 1]];
					const Image& img_2 = range_images[range_idx[j]];
					if (img_1.data && img_2.data 
						&& (img_1.width == img_2.width & img_1.height == img_2.height & img_1.nchannels == img_2.nchannels))
					{
						duplicate_images.push_back(sorted_idx[range_idx[j] + start_range]);
						// If the dimensions are same, check the data
						// if (std::equal(img_1.data, img_1.data + img_1.width * img_1.height, img_2.data))
						// {
						// 	// Images are same, skip loop
						// 	std::cout << dir_list[sorted_idx[range_idx[j] + start_range]] << " === " 
						// 		<< dir_list[sorted_idx[range_idx[j - 1] + start_range]] << "\n";
						// 	duplicate_images.push_back(sorted_idx[range_idx[j] + start_range]);
						// }
					}
				}
				for (int j = 0; j < end_range - start_range + 1; ++j)
				{
					stbi_image_free(range_images[j].data);
				}
			}
			start_range = -1;
			end_range = -1;
		}
	}

	return duplicate_images;
}

template <bool remove>
void RemoveDuplicates(const DirList& dir_list, const std::vector<int>& duplicates)
{
	for (const int ind : duplicates)
	{
		if constexpr (remove)
		{
			std::cout << "Removing " << dir_list[ind] << "\n";
			fs::remove(dir_list[ind]);
		}
		else
		{
			std::cout << "Not removing duplicate " << dir_list[ind] << "\n";
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cout << "Usage: RemoveDuplicates.exe image_folder remove/noremove";
		return 1;
	}
	const char* root_dir = argv[1];

	DirList dir_list = ListDir(root_dir, EXT_FILTERS);
	std::vector<int> duplicates = FindDuplicates(dir_list);

	if (!strcmp(argv[2], "remove"))
	{
		RemoveDuplicates<true>(dir_list, duplicates);
	}
	else
	{
		RemoveDuplicates<false>(dir_list, duplicates);
	}

	return 0;
}
