
import type { Metadata } from "next";
import { Inter } from "next/font/google";
import SiteFooter from "./components/SiteFooter";
import "./globals.css";

const inter = Inter({
  subsets: ["latin"],
  variable: "--font-inter",
  display: "swap",
});

const siteUrl = "https://welookit.com";

export const metadata: Metadata = {
  metadataBase: new URL(siteUrl),

  title: {
    default: "Welookit - Company Intelligence & Website Research",
    template: "%s | Welookit",
  },

  description:
    "Explore company intelligence from any website. Discover public contact details, social profiles, leadership information, website technologies and digital presence with Welookit.",

  applicationName: "Welookit",

  category: "Business",

  keywords: [
    "company intelligence",
    "company research",
    "website research",
    "website technology checker",
    "technology stack detection",
    "company information",
    "business research",
    "public contact discovery",
    "website analysis",
    "digital presence research",
  ],

  alternates: {
    canonical: "/",
  },

  robots: {
    index: true,
    follow: true,
    googleBot: {
      index: true,
      follow: true,
      "max-image-preview": "large",
      "max-snippet": -1,
      "max-video-preview": -1,
    },
  },

  openGraph: {
    type: "website",
    locale: "en_US",
    url: siteUrl,
    siteName: "Welookit",
    title: "Welookit - Company Intelligence & Website Research",
    description:
      "Explore company information, public contacts, social profiles, leadership and website technologies in one place.",
  },

  twitter: {
    card: "summary_large_image",
    title: "Welookit - Company Intelligence & Website Research",
    description:
      "Discover company information, public contacts, social profiles and website technologies with Welookit.",
  },
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body className={inter.variable}>
        {children}
        <SiteFooter />
      </body>
    </html>
  );
}
