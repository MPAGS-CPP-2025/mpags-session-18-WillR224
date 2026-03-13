#include "CaesarCipher.hpp"
#include "Alphabet.hpp"
#include <future>
#include <chrono>
#include <string>

/**
 * \file CaesarCipher.cpp
 * \brief Contains the implementation of the CaesarCipher class
 */

CaesarCipher::CaesarCipher(const std::size_t key) : key_{key % Alphabet::size}
{
}

CaesarCipher::CaesarCipher(const std::string& key) : key_{0}
{
    // We have the key as a string, but the Caesar cipher needs an unsigned long, so we first need to convert it
    // We default to having a key of 0, i.e. no encryption, if no (valid) key was provided on the command line
    if (!key.empty()) {
        // First, explicitly check for negative numbers - these will convert successfully but will not lead to expected results
        if (key.front() == '-') {
            throw InvalidKey(
                "Caesar cipher requires a positive long integer key, the supplied key (" +
                key + ") could not be successfully converted");
        }
        // The conversion function will throw one of two possible exceptions
        // if the string does not represent a valid unsigned long integer
        try {
            key_ = std::stoul(key) % Alphabet::size;
        } catch (const std::invalid_argument&) {
            throw InvalidKey(
                "Caesar cipher requires a positive long integer key, the supplied key (" +
                key + ") could not be successfully converted");
        } catch (const std::out_of_range&) {
            throw InvalidKey(
                "Caesar cipher requires a positive long integer key, the supplied key (" +
                key + ") could not be successfully converted");
        }
    }
}

std::string CaesarCipher::applyCipher(const std::string& inputText,
                                      const CipherMode cipherMode) const
{
    // Create the output string
    std::string outputText;
    int threadCount{5};
    std::vector<std::future<std::string>> futures;
    std::string inputTextChunk;
    for(int i{0}; i < threadCount; ++i) {
        int chunkSize = inputText.size() / threadCount;
        int start = i * chunkSize;
        int end;
        if(i == threadCount -1){
            end = inputText.size();
        }
        else{
            end = (i + 1) * chunkSize;
        }

        auto processChunk = [this, cipherMode](const std::string& textChunk) {
            // Loop over the input text
            std::string outputChunk;
            char processedChar{'x'};
            for (const auto& origChar : textChunk) {
                // For each character in the input text, find the corresponding position in
                // the alphabet by using an indexed loop over the alphabet container
                for (std::size_t j{0}; j < Alphabet::size; ++j) {
                    if (origChar == Alphabet::alphabet[j]) {
                        // Apply the appropriate shift (depending on whether we're encrypting
                        // or decrypting) and determine the new character
                        // Can then break out of the loop over the alphabet
                        switch (cipherMode) {
                            case CipherMode::Encrypt:
                                processedChar =
                                    Alphabet::alphabet[(j + key_) % Alphabet::size];
                                break;
                            case CipherMode::Decrypt:
                                processedChar =
                                    Alphabet::alphabet[(j + Alphabet::size - key_) %
                                                    Alphabet::size];
                                break;
                        }
                        break;
                    }
                }
                // Add the new character to the output text
                outputChunk += processedChar;
            }
            return outputChunk;
        };

        futures.push_back(std::async(std::launch::async, processChunk, inputText.substr(start, end - start)));


    }

    for (auto& future : futures) {
        std::future_status status{std::future_status::ready};
        do {
            status = future.wait_for(std::chrono::seconds(1));
        } 
        while (status != std::future_status::ready);
        outputText += future.get();
    }


    return outputText;
}
