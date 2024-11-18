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
    NO
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
int get_int(fstream& stream, int offset, int bytes)
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
vector<vector<Pixel>> read_image(string filename)
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
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

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
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
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
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

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
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


// Process 1
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

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

            Pixel new_pixel;
            new_pixel.red = new_red;
            new_pixel.green = new_green;
            new_pixel.blue = new_blue;

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

// Process 2
vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

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

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
            else if (average_value <= 90)
            {
                int new_red = red_value * scaling_factor;
                int new_green = green_value * scaling_factor;
                int new_blue = blue_value * scaling_factor;

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
            else
            {
                int new_red = red_value;
                int new_green = green_value;
                int new_blue = blue_value;

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
        }
    }

    return new_image;
}

// Process 3
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

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

            Pixel new_pixel;
            new_pixel.red = new_red;
            new_pixel.green = new_green;
            new_pixel.blue = new_blue;

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

// Process 4
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));

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
vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)
{
    int angle = number * 90;

    if (angle % 90 != 0)
    {
        cout << "angle must be a multiple of 90 degrees." << endl;
    }

    int rotation = (angle % 360) / 90.0;
    vector<vector<Pixel>> new_image = image;

    for (int i = 0; i < rotation; i++)
    {
        new_image = process_4(new_image);
    }

    return new_image;
}

// Process 6
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale)
{
    int height = image.size();
    int width = image[0].size();

    int new_height = y_scale * height;
    int new_width = x_scale * width;

    vector<vector<Pixel>> new_image(new_height, vector<Pixel>(new_width));

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
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

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

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            } else
            {
                int new_red = 0;
                int new_green = 0;
                int new_blue = 0;

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
        }
    }

    return new_image;
}

// Process 8
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            int new_red = (255 - (255 - red_value) * scaling_factor);
            int new_green = (255 - (255 - green_value) * scaling_factor);
            int new_blue = (255 - (255 - blue_value) * scaling_factor);

            Pixel new_pixel;
            new_pixel.red = new_red;
            new_pixel.green = new_green;
            new_pixel.blue = new_blue;

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

// Process 9
vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int red_value = image[row][col].red;
            int green_value = image[row][col].green;
            int blue_value = image[row][col].blue;

            int new_red = red_value * scaling_factor;
            int new_green = green_value * scaling_factor;
            int new_blue = blue_value * scaling_factor;

            Pixel new_pixel;
            new_pixel.red = new_red;
            new_pixel.green = new_green;
            new_pixel.blue = new_blue;

            new_image[row][col] = new_pixel;
        }
    }

    return new_image;
}

// Process 10
vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));

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

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
            else if (red_value + green_value + blue_value <= 150)
            {
                int new_red = 0;
                int new_green = 0;
                int new_blue = 0;

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
            else if (max_color == red_value)
            {
                int new_red = 255;
                int new_green = 0;
                int new_blue = 0;

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
            else if (max_color == green_value)
            {
                int new_red = 0;
                int new_green = 255;
                int new_blue = 0;

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
            else
            {
                int new_red = 0;
                int new_green = 0;
                int new_blue = 255;

                Pixel new_pixel;
                new_pixel.red = new_red;
                new_pixel.green = new_green;
                new_pixel.blue = new_blue;

                new_image[row][col] = new_pixel;
            }
        }
    }

    return new_image;
}

// Run menu UI
string menu(string filename)
{
    cout << "" << endl;
    cout << "IMAGE PROCESSING MENU" << endl;
    cout << "" << endl;
    cout << " 0) Change Image (current: " << filename << ")" << endl;
    cout << " 1) Vignette" << endl;
    cout << " 2) Clarendon" << endl;
    cout << " 3) Grayscale" << endl;
    cout << " 4) Rotate 90 Degrees" << endl;
    cout << " 5) Rotate Multiple 90 Degrees" << endl;
    cout << " 6) Enlarge" << endl;
    cout << " 7) High Contrast" << endl;
    cout << " 8) Lighten" << endl;
    cout << " 9) Darken" << endl;
    cout << "10) Black, White, Red, Green, Blue" << endl;
    cout << "" << endl;
    cout << "Make a selection (Q to quit): ";

    string selection;
    cin >> selection;

    return selection;
}

// Verify filename is .bmp
string get_valid_filename(string prompt)
{
    string filename;
    while (true)
    {
        cout << prompt;
        cin >> filename;
        if (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".bmp")
        {
            return filename;
        }
        cout << "Error: Input file must be a .bmp file." << endl;
    }
}

// Verify output filename is .bmp, and not the same as input filename or an already saved output filename
string get_output_filename(string input_filename, vector<string> existing_outputs, string prompt)
{
    string output_filename;
    while (true)
    {
        cout << prompt;
        cin >> output_filename;
        if (output_filename.length() < 4 || output_filename.substr(output_filename.length() -4) != ".bmp")
        {
            cout << "Error: Output file must be a .bmp file." << endl;
        }
        else if (output_filename == input_filename)
        {
            cout << "Error: Output filename cannot be the same as the input filename." << endl;
        }
        else if (find(existing_outputs.begin(), existing_outputs.end(), output_filename) != existing_outputs.end())
        {
            cout << "Error: Output filename already exists." << endl;
        }
        else
        {
            return output_filename;
        }
    }
}

// Ask for and verify number
int get_valid_number(string prompt, int min_value)
{
    int value;
    while (true)
    {
        cout << prompt;
        if (cin >> value && value >= min_value)
        {
            return value;
        }
        cout << "Error: Please enter a number greater than or equal to " << min_value << "." << endl;
        cin.clear();
        cin.ignore(1000, '\n');
    }
}

// Ask for and verify scaling factor
double get_valid_scaling_factor(string prompt, double min_value, double max_value)
{
    double value;
    while (true)
    {
        cout << prompt;
        if (cin >> value && value > min_value && value < max_value)
        {
            return value;
        }
        cout << "Error: Please enter a number between " << min_value << " and " << max_value << "." << endl;
        cin.clear();
        cin.ignore(1000, '\n');
    }
}



int main()
{
    cout << "" << endl;
    cout << "CSPB 1300 Image Processing Application" << endl;
    cout << "" << endl;
    cout << "Please enter a filename (.bmp only): ";
    string filename;
    cin >> filename;

    if (filename.substr(filename.length() - 4) != ".bmp")
    {
        cout << "Error: Input file must be a .bmp file." << endl;
        main();
    }

    vector<vector<Pixel>> image = read_image(filename);
    vector<vector<Pixel>> new_image;
    vector<string> output_filenames;

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
        }

        if (selection == "Q" || selection == "q")
        {
            cout << "" << endl;
            cout << "Quitting program....Goodbye!" << endl;
            cout << "" << endl;
            break;
        }

        // UI if user selects option "0"
        if (selection == "0")
        {
            filename = get_valid_filename("Please enter a filename (.bmp only): ");
            image = read_image(filename);
        }

        // UI if user selects option "1"
        else if (selection == "1")
        {
            cout << "" << endl;
            cout << "Vignette selected" << endl;
            cout << "" << endl;
            vignette_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(vignette_output);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_1(image);
            write_image(vignette_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied vignette and saved to " << vignette_output << "!" << endl;
        }

        // UI if user selects option "2"
        else if (selection == "2")
        {
            cout << "" << endl;
            cout << "Clarendon selected" << endl;
            cout << "" << endl;
            clarendon_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(clarendon_output);

            double scaling_factor = get_valid_scaling_factor("Enter scaling factor: ", 0.0, 1.0);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_2(image, scaling_factor);
            write_image(clarendon_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied clarendon and saved to " << clarendon_output << "!" << endl;
        }

        // UI if user selects option "3"
        else if (selection == "3")
        {
            cout << "" << endl;
            cout << "Grayscale selected" << endl;
            cout << "" << endl;
            grayscale_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(grayscale_output);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_3(image);
            write_image(grayscale_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied grayscale and saved to " << grayscale_output << "!" << endl;
        }

        // UI if user selects option "4"
        else if (selection == "4")
        {
            cout << "" << endl;
            cout << "Rotate 90 Degrees selected" << endl;
            cout << "" << endl;
            rotate_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(vignette_output);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_4(image);
            write_image(rotate_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied rotate 90 degrees and saved to " << rotate_output << "!" << endl;
        }

        // UI if user selects option "5"
        else if (selection == "5")
        {
            cout << "" << endl;
            cout << "Rotate 90 Degrees selected" << endl;
            cout << "" << endl;
            rotate_multiple_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(rotate_multiple_output);

            int number = get_valid_number("Enter a number: ", 1);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_5(image, number);
            write_image(rotate_multiple_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied rotate 90 degrees and saved to " << rotate_multiple_output << "!" << endl;
        }

        // UI if user selects option "6"
        else if (selection == "6")
        {
            cout << "" << endl;
            cout << "Enlarge selected" << endl;
            cout << "" << endl;
            enlarge_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(enlarge_output);

            int x_scale = get_valid_number("Enter a number: ", 1);
            int y_scale = get_valid_number("Enter another number: ", 1);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_6(image, x_scale, y_scale);
            write_image(enlarge_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied enlarge and saved to " << enlarge_output << "!" << endl;
        }

        // UI if user selects option "7"
        else if (selection == "7")
        {
            cout << "" << endl;
            cout << "High Contrast selected" << endl;
            cout << "" << endl;
            contrast_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(contrast_output);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_7(image);
            write_image(contrast_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied high contrast and saved to " << contrast_output << "!" << endl;
        }

        // UI if user selects option "8"
        else if (selection == "8")
        {
            cout << "" << endl;
            cout << "Lighten selected" << endl;
            cout << "" << endl;
            lighten_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(lighten_output);

            double scaling_factor = get_valid_scaling_factor("Enter scaling factor: ", 0.0, 1.0);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_8(image, scaling_factor);
            write_image(lighten_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied lighten and saved to " << lighten_output << "!" << endl;
        }

        // UI if user selects option "9"
        else if (selection == "9")
        {
            cout << "" << endl;
            cout << "Darken selected" << endl;
            cout << "" << endl;
            darken_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(darken_output);

            double scaling_factor = get_valid_scaling_factor("Enter scaling factor: ", 0.0, 1.0);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_9(image, scaling_factor);
            write_image(darken_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied darken and saved to " << darken_output << "!" << endl;
        }

        // UI if user selects option "10"
        else if (selection == "10")
        {
            cout << "" << endl;
            cout << "Black, White, Red, Green, Blue selected" << endl;
            cout << "" << endl;
            color_output = get_output_filename(filename, output_filenames, "Enter output filename (.bmp only): ");
            output_filenames.push_back(color_output);

            // Runs the proper process and writes the image to user provided output file.
            new_image = process_10(image);
            write_image(color_output, new_image);
            cout << "" << endl;
            cout << "Successfully applied colors and saved to " << color_output << "!" << endl;
        }
    }

    return 0;
}