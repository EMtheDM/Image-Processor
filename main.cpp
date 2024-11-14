/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Eric Martin

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:

*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */
int get_int(fstream &stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel> > read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel> > image(height, vector<Pixel>(width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset + i] = (unsigned char) (value >> (i * 8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel> > &image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header, 0, 1, 'B'); // ID field
    set_bytes(bmp_header, 1, 1, 'M'); // ID field
    set_bytes(bmp_header, 2, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE + array_bytes); // Size of BMP file
    set_bytes(bmp_header, 6, 2, 0); // Reserved
    set_bytes(bmp_header, 8, 2, 0); // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header, 0, 4, DIB_HEADER_SIZE); // DIB header size
    set_bytes(dib_header, 4, 4, width_pixels); // Width of bitmap in pixels
    set_bytes(dib_header, 8, 4, height_pixels); // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1); // Number of color planes
    set_bytes(dib_header, 14, 2, 24); // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0); // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes); // Size of raw bitmap data (including padding)
    set_bytes(dib_header, 24, 4, 2835); // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835); // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0); // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0); // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char *) bmp_header, sizeof(bmp_header));
    stream.write((char *) dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char *) pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *) padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

// Process 1
vector<vector<Pixel> > process_1(const vector<vector<Pixel> > &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            double distance = sqrt(pow(col - width / 2, 2) + pow(row - height / 2, 2));
            double scaling_factor = (height - distance) / height;

            int new_red = red_value * scaling_factor;
            int new_green = green_value * scaling_factor;
            int new_blue = blue_value * scaling_factor;

            new_image[row][col] = Pixel(new_red, new_green, new_blue);
        }
    }

    return new_image;
}

// Process 2
vector<vector<Pixel> > process_2(const vector<vector<Pixel> > &image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            double average_value = (red_value + green_value + blue_value) / 3.0;

            if (average_value >= 170)
            {
                int new_red = (255 - (255 - red_value) * scaling_factor);
                int new_green = (255 - (255 - green_value) * scaling_factor);
                int new_blue = (255 - (255 - blue_value) * scaling_factor);

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            } else if (average_value <= 90)
            {
                int new_red = red_value * scaling_factor;
                int new_green = green_value * scaling_factor;
                int new_blue = blue_value * scaling_factor;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            } else
            {
                int new_red = red_value;
                int new_green = green_value;
                int new_blue = blue_value;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            }
        }
    }

    return new_image;
}

// Process 3
vector<vector<Pixel> > process_3(const vector<vector<Pixel> > &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            double gray_value = (red_value + green_value + blue_value) / 3.0;

            int new_red = gray_value;
            int new_green = gray_value;
            int new_blue = gray_value;

            new_image[row][col] = Pixel(new_red, new_green, new_blue);
        }
    }

    return new_image;
}

// Process 4
vector<vector<Pixel> > process_4(const vector<vector<Pixel> > &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(width, vector<Pixel>(height));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            new_image[col][(height - 1) - row] = image[row][col];
        }
    }

    return new_image;
}

// Process 5
vector<vector<Pixel> > process_5(const vector<vector<Pixel> > &image, int number)
{
    int angle = number * 90;

    if (angle % 90 != 0)
    {
        cout << "angle must be a multiple of 90 degrees." << endl;
    } else if (angle % 360 == 0)
    {
        return image;
    } else if (angle % 360 == 90)
    {
        return process_4(image);
    } else if (angle % 360 == 180)
    {
        return process_4(process_4(image));
    } else
    {
        return process_4(process_4(process_4(image)));
    }
}

// Process 6
vector<vector<Pixel> > process_6(const vector<vector<Pixel> > &image, int x_scale, int y_scale)
{
    int height = image.size();
    int width = image[0].size();

    int new_height = y_scale * height;
    int new_width = x_scale * width;

    vector<vector<Pixel> > new_image(new_height, vector<Pixel>(new_width));

    for (int row = 0; row < new_height; row++)
    {
        for (int col = 0; col < new_width; col++)
        {
            int original_x_scale = col / x_scale;
            int original_y_scale = row / y_scale;

            original_x_scale = min(original_x_scale, width - 1);
            original_y_scale = min(original_y_scale, height - 1);

            new_image[row][col] = image[original_y_scale][original_x_scale];
        }
    }

    return new_image;
}

// Process 7
vector<vector<Pixel> > process_7(const vector<vector<Pixel> > &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            double gray_value = (red_value + green_value + blue_value) / 3.0;

            if (gray_value >= 255 / 2.0)
            {
                int new_red = 255;
                int new_green = 255;
                int new_blue = 255;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            } else
            {
                int new_red = 0;
                int new_green = 0;
                int new_blue = 0;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            }
        }
    }

    return new_image;
}

// Process 8
vector<vector<Pixel> > process_8(const vector<vector<Pixel> > &image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            double new_red = (255 - (255 - red_value) * scaling_factor);
            double new_green = (255 - (255 - green_value) * scaling_factor);
            double new_blue = (255 - (255 - blue_value) * scaling_factor);

            new_image[row][col] = Pixel(new_red, new_green, new_blue);
        }
    }

    return new_image;
}

// Process 9
vector<vector<Pixel> > process_9(const vector<vector<Pixel> > &image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            double new_red = red_value * scaling_factor;
            double new_green = green_value * scaling_factor;
            double new_blue = blue_value * scaling_factor;

            new_image[row][col] = Pixel(new_red, new_green, new_blue);
        }
    }

    return new_image;
}

// Process 10
vector<vector<Pixel> > process_10(const vector<vector<Pixel> > &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel> > new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            int max_color = max(red_value, max(green_value, blue_value));

            if (red_value + green_value + blue_value >= 550)
            {
                int new_red = 255;
                int new_green = 255;
                int new_blue = 255;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            } else if (red_value + green_value + blue_value <= 150)
            {
                int new_red = 0;
                int new_green = 0;
                int new_blue = 0;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            } else if (max_color == red_value)
            {
                int new_red = 255;
                int new_green = 0;
                int new_blue = 0;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            } else if (max_color == green_value)
            {
                int new_red = 0;
                int new_green = 255;
                int new_blue = 0;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            } else
            {
                int new_red = 0;
                int new_green = 0;
                int new_blue = 255;

                new_image[row][col] = Pixel(new_red, new_green, new_blue);
            }
        }
    }

    return new_image;
}

string menu(string filename)
{
    cout << "IMAGE PROCESSING MENU" << endl;
    cout << "0) Change Image (current: " << filename << ")" << endl;
    cout << "1) Vignette" << endl;
    cout << "2) Clarendon" << endl;
    cout << "3) Grayscale" << endl;
    cout << "4) Rotate 90 Degrees" << endl;
    cout << "5) Rotate Multiple 90 Degrees" << endl;
    cout << "6) Enlarge" << endl;
    cout << "7) High Contrast" << endl;
    cout << "8) Lighten" << endl;
    cout << "9) Darken" << endl;
    cout << "10) Black, White, Red, Green, Blue" << endl;
    cout << "" << endl;
    cout << "Make a selection (Q to quit): ";

    string selection;
    cin >> selection;

    return selection;
}

bool is_number(const string &str)
{
    for (char c: str)
    {
        if (!isdigit(c))
        {
            return false;
        }
    }
    return true;
}


int main()
{
    cout << "CSPB 1300 Image Processing Application" << endl;
    cout << "" << endl;
    cout << "Please enter a filename (.bmp only): ";
    string filename;
    cin >> filename;

    if (filename.substr(filename.length() - 4) != ".bmp")
    {
        cout << "Error: Input file must be a .bmp file." << endl;
        return 1;
    }

    vector<vector<Pixel> > image = read_image(filename);
    vector<vector<Pixel> > new_image;

    string selection;
    string vignette_output;
    string clarendon_output;
    string grayscale_output;
    string rotate_output;
    string rotate_multiple_output;
    string enlarge_output;
    string contrast_output;
    string lighten_output;
    string darken_output;
    string color_output;

    while (true)
    {
        selection = menu(filename);

        if (selection != "0" && selection != "1" && selection != "2" && selection != "3" && selection != "4" &&
            selection
            != "5" && selection != "6" && selection != "7" && selection != "8" && selection != "9" && selection != "10"
            &&
            selection != "Q" && selection != "q")
        {
            cout << "Error. Input must be between 0-10 or Q/q to quit." << endl;
            return 1;
        }

        if (selection == "Q" || selection == "q")
        {
            cout << "Quitting program....Goodbye!" << endl;
            break;
        }

        // UI if user selects option "0"
        if (selection == "0")
        {
            cout << "Please enter a filename (.bmp only): ";
            cin >> filename;

            // Checks to make sure user selects .bmp file.
            if (filename.substr(filename.length() - 4) != ".bmp")
            {
                cout << "Error: Input file must be a .bmp file." << endl;
                return 1;
            }

            image = read_image(filename);
        }

        // UI if user selects option "1"
        else if (selection == "1")
        {
            cout << "Vignette selected" << endl;
            cout << "Enter output filename (.bmp only): ";
            cin >> vignette_output;

            // Checks to make sure user sets output to .bmp file.
            if (vignette_output.substr(vignette_output.length() - 4) != ".bmp")
            {
                cout << "Error: Output file must be a .bmp file." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as input which would override input.
            if (vignette_output == filename)
            {
                cout << "Error: output filename cannot be the same as input filename." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as another output which would override that output.
            if (vignette_output == grayscale_output || vignette_output == clarendon_output || vignette_output ==
                rotate_output || vignette_output == rotate_multiple_output || vignette_output == enlarge_output ||
                vignette_output == contrast_output || vignette_output == lighten_output || vignette_output ==
                darken_output || vignette_output == color_output)
            {
                cout << "Error: current output filename cannot be the same as another output filename." << endl;
                cout << "Failed to write vignette image." << endl;
                return 1;
            }

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_1(image);
            write_image(vignette_output, new_image);
            cout << "Successfully applied vignette and saved to " << vignette_output << "!" << endl;
        }

        // UI if user selects option "2"
        else if (selection == "2")
        {
            cout << "Clarendon selected" << endl;
            cout << "Enter output filename (.bmp only): ";
            cin >> clarendon_output;

            // Checks to make sure user sets output to .bmp file.
            if (clarendon_output.substr(clarendon_output.length() - 4) != ".bmp")
            {
                cout << "Error: Output file must be a .bmp file." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as input which would override input.
            if (clarendon_output == filename)
            {
                cout << "Error: output filename cannot be the same as input filename." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as another output which would override that output.
            if (clarendon_output == vignette_output || clarendon_output == grayscale_output || clarendon_output ==
                rotate_output || clarendon_output == rotate_multiple_output || clarendon_output == enlarge_output ||
                clarendon_output == contrast_output || clarendon_output == lighten_output || clarendon_output ==
                darken_output || clarendon_output == color_output)
            {
                cout << "Error: current output filename cannot be the same as another output filename." << endl;
                cout << "Failed to write clarendon image." << endl;
                return 1;
            }

            cout << "Enter scaling factor: ";
            double scaling_factor;
            cin >> scaling_factor;

            // Checks to make sure scaling_factor is a number greater than 0.
            if (scaling_factor < 0)
            {
                cout << "Error: scaling factor must be greater than 0." << endl;
                return 1;
            }

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_2(image, scaling_factor);
            write_image(clarendon_output, new_image);
            cout << "Successfully applied clarendon and saved to " << clarendon_output << "!" << endl;
        }

        // UI if user selects option "3"
        else if (selection == "3")
        {
            cout << "Grayscale selected" << endl;
            cout << "Enter output filename (.bmp only): ";
            cin >> grayscale_output;

            // Checks to make sure user sets output to .bmp file.
            if (grayscale_output.substr(grayscale_output.length() - 4) != ".bmp")
            {
                cout << "Error: Output file must be a .bmp file." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as input which would override input.
            if (grayscale_output == filename)
            {
                cout << "Error: output filename cannot be the same as input filename." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as another output which would override that output.
            if (grayscale_output == vignette_output || grayscale_output == clarendon_output || grayscale_output ==
                rotate_output || grayscale_output == rotate_multiple_output || grayscale_output == enlarge_output ||
                grayscale_output == contrast_output || grayscale_output == lighten_output || grayscale_output ==
                darken_output || grayscale_output == color_output)
            {
                cout << "Error: current output filename cannot be the same as another output filename." << endl;
                cout << "Failed to write grayscale image." << endl;
                return 1;
            }

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_3(image);
            write_image(grayscale_output, new_image);
            cout << "Successfully applied grayscale and saved to " << grayscale_output << "!" << endl;
        }

        // UI if user selects option "4"
        else if (selection == "4")
        {
            cout << "Rotate 90 Degrees selected" << endl;
            cout << "Enter output filename (.bmp only): ";
            cin >> rotate_output;

            // Checks to make sure user sets output to .bmp file.
            if (rotate_output.substr(rotate_output.length() - 4) != ".bmp")
            {
                cout << "Error: Output file must be a .bmp file." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as input which would override input.
            if (rotate_output == filename)
            {
                cout << "Error: output filename cannot be the same as input filename." << endl;
                return 1;
            }

            // Checks to make sure user doesn't title output the same as another output which would override that output.
            if (rotate_output == vignette_output || rotate_output == clarendon_output || rotate_output ==
                grayscale_output || rotate_output == rotate_multiple_output || rotate_output == enlarge_output ||
                rotate_output == contrast_output || rotate_output == lighten_output || rotate_output ==
                darken_output || rotate_output == color_output)
            {
                cout << "Error: current output filename cannot be the same as another output filename." << endl;
                cout << "Failed to write rotate 90 degrees image." << endl;
                return 1;
            }

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_3(image);
            write_image(rotate_output, new_image);
            cout << "Successfully applied rotate 90 degrees and saved to " << rotate_output << "!" << endl;
        }
    }

    return 0;
}
