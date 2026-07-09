import SwiftUI

struct ContentView: View {
    @State private var isAuthenticating = false
    @State private var authErrorMessage: String?

    var body: some View {
        VStack(spacing: 10) {
            HStack {
                Image(systemName: "waveform")
                    .imageScale(.large)
                    .foregroundStyle(.tint)
                Text("Welcome to Echoes")
            }

            Button("Connect OneDrive") {
                startAuthentication()
            }
            .disabled(isAuthenticating)

            if let authErrorMessage {
                Text(authErrorMessage)
                    .foregroundStyle(.red)
            }
        }
        .padding()
    }

    private func startAuthentication() {
        isAuthenticating = true
        authErrorMessage = nil

        let startUrl = ProviderBridge.start_authentication(.oneDrive) { success, errorMessage in
            // Already hopped back to the main queue on the Objective-C++ side.
            isAuthenticating = false
            authErrorMessage = success ? nil : (errorMessage ?? "Unknown error")
        }

        if let url = URL(string: startUrl) {
            NSWorkspace.shared.open(url)
        }
    }
}

#Preview {
    ContentView()
}
