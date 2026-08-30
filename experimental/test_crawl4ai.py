#!/usr/bin/env python3
"""Crawl4AI Test Script for Codebase Indexing."""

import asyncio
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent / ".venv" / "lib" / "python3.12" / "site-packages"))

from crawl4ai import AsyncWebCrawler, CrawlerRunConfig, CacheMode


async def test_web_crawl():
    print("=" * 60)
    print("TEST 1: Web URL Crawling (verify installation)")
    print("=" * 60)
    
    config = CrawlerRunConfig(cache_mode=CacheMode.BYPASS, verbose=True)
    
    async with AsyncWebCrawler() as crawler:
        result = await crawler.arun(url="https://example.com", config=config)
        
        if result.success:
            print(f"✓ Crawl successful!")
            print(f"  URL: {result.url}")
            print(f"  Status: {result.status_code}")
            print(f"  Content length: {len(result.markdown)} chars")
            print(f"\n  First 500 chars of markdown:")
            print("-" * 40)
            print(result.markdown[:500])
            return True
        else:
            print(f"✗ Crawl failed: {result.error_message}")
            return False


async def test_file_url():
    print("\n" + "=" * 60)
    print("TEST 2: Local File via file:// URL")
    print("=" * 60)
    
    readme_path = Path("/home/jens/git/xtcw2/README.md").resolve()
    file_url = f"file://{readme_path}"
    
    print(f"  Trying: {file_url}")
    
    config = CrawlerRunConfig(cache_mode=CacheMode.BYPASS, verbose=True)
    
    async with AsyncWebCrawler() as crawler:
        try:
            result = await crawler.arun(url=file_url, config=config)
            
            if result.success:
                print(f"✓ File crawl successful!")
                print(f"  Content length: {len(result.markdown)} chars")
                print(f"\n  Content preview:")
                print("-" * 40)
                print(result.markdown[:500])
                return True
            else:
                print(f"✗ File crawl failed: {result.error_message}")
                return False
        except Exception as e:
            print(f"✗ Exception: {type(e).__name__}: {e}")
            return False


async def test_markdown_generation():
    print("\n" + "=" * 60)
    print("TEST 3: Markdown Generation Quality")
    print("=" * 60)
    
    config = CrawlerRunConfig(cache_mode=CacheMode.BYPASS, verbose=True)
    
    async with AsyncWebCrawler() as crawler:
        result = await crawler.arun(url="https://httpbin.org/html", config=config)
        
        if result.success:
            print(f"✓ Markdown generation working")
            print(f"  Content type: {type(result.markdown)}")
            print(f"\n  Sample output:")
            print("-" * 40)
            print(result.markdown[:300])
            return True
        else:
            print(f"✗ Failed: {result.error_message}")
            return False


async def main():
    print("Crawl4AI Test Suite")
    print("=" * 60)
    print(f"Python version: {sys.version}")
    print()
    
    results = []
    results.append(("Web Crawling", await test_web_crawl()))
    results.append(("File URL", await test_file_url()))
    results.append(("Markdown Generation", await test_markdown_generation()))
    
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    for name, success in results:
        status = "✓ PASS" if success else "✗ FAIL"
        print(f"  {status}: {name}")
    
    all_passed = all(success for _, success in results)
    print()
    if all_passed:
        print("All tests passed!")
    else:
        print("Some tests failed - Crawl4AI works for web, not local files")
    
    return 0 if all_passed else 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)
